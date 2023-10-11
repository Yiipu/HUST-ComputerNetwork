#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <winsock2.h>
#include <unordered_map>
#include <ctime>
using namespace std;

#pragma comment(lib, "ws2_32.lib")

// 配置参数数据结构
struct ServerConfig {
    char IP[256];
    int PORT;
    char HOME[256];
    int WaitCapcity;
};

// 结构体存储HTTP请求解析结果
struct ParsedRequest {
    string method;
    string uri;
    string httpVersion;
};

// ContentType
unordered_map<string, string> contentTypes = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".gif", "image/gif"},
    {".png", "image/png"},
    {".jpg", "image/jpeg"},
    {".mp3", "audio/mp3"},
    {".mp4", "audio/mp4"},
    {".ico", "image/x-icon"},
    {".pdf", "application/pdf"},
    {".zip", "application/zip"},
    {".txt", "text/plain"},
    // 添加其他文件类型
};

// 存储旧URL与新URL，用以判断403
unordered_map<string, string> UrlMap = {
    {"/moved.html", "/moved/moved.html"},
    // 添加其他URL映射
};

// 写日志
void WriteLog(string Ip, int Port, string note, string path) {
    ofstream logFile("./log.txt", ios::app);
    if (logFile.is_open()) {
        time_t currentTime = time(nullptr);
        tm* localTime = localtime(&currentTime);
        logFile << "When " 
            << localTime->tm_year + 1900 << "-" 
            << localTime->tm_mon + 1 << "-"
            << localTime->tm_mday << " " 
            << localTime->tm_hour << ":" 
            << localTime->tm_min << ":"
            << localTime->tm_sec << ",";
        logFile << "IP: " << Ip << " Port: " << Port << " Try to get the source from " << path << ", but " << note << " occurred" << endl;
        logFile.close();
    }
}

// 解析HTTP请求报文
ParsedRequest ParseHttpRequest(const char* request) {
    ParsedRequest parsedRequest;
    istringstream requestStream(request);

    requestStream >> parsedRequest.method >> parsedRequest.uri >> parsedRequest.httpVersion;
    // 打印解析结果
    cout << "Request Method: " << parsedRequest.method << endl;
    cout << "Request URI: " << parsedRequest.uri << endl;
    cout << "HTTP Version: " << parsedRequest.httpVersion << endl;
    return parsedRequest;
}

// 返回错误信息并关闭连接
void SendErrorResponse(SOCKET clientSocket, const sockaddr_in& clientAddr, int errorCode, const string& path) {
    // 头部
    string code;
    string contentType;

    switch (errorCode) {
    case 400:
        code = "400 Bad Request";
        contentType = contentTypes[".html"];
        break;
    case 404:
        code = "404 Not Found";
        contentType = contentTypes[".html"];
        break;
    case 301:
        code = "301 Moved Permanently\r\nlocation:" + UrlMap[path];
        contentType = contentTypes[".txt"];
        break;
        // 添加其他错误代码
    default:
        code = "500 Internal Server Error";
        contentType = contentTypes[".html"];
        break;
    }

    cout << "Response: " << code << endl;
    WriteLog(inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), code, path);

    string ResponseHeader = "HTTP/1.1 " + code + "\r\nContent-type: " + contentType + "\r\n\r\n";
    send(clientSocket, ResponseHeader.c_str(), ResponseHeader.length(), 0);
    closesocket(clientSocket);
}


// 处理HTTP请求
void HandleHttpRequest(SOCKET clientSocket, const ServerConfig& serverConfig) {
    // 接收从客户端发送的HTTP请求数据
    char buffer[4096]; // 缓冲区
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead == SOCKET_ERROR) {
        cerr << "Receive data failed!" << endl;
        closesocket(clientSocket);
        return;
    }
    buffer[bytesRead] = '\0'; // EOF

    // 获取客户端 IP 及 PORT
    sockaddr_in clientAddr;
    int addrLen = sizeof(clientAddr);
    getpeername(clientSocket, (struct sockaddr*)&clientAddr, &addrLen);

    // 打印 IP 及 PORT
    std::cout << "Client IP: " << inet_ntoa(clientAddr.sin_addr) << std::endl;
    std::cout << "Client Port: " << ntohs(clientAddr.sin_port) << std::endl;

    // 解析HTTP请求并获取请求的文件路径
    ParsedRequest parsedRequest = ParseHttpRequest(buffer);
    string pathStr = parsedRequest.uri;

    string ResponseHeader;
    // 过滤请求方式, 只允许 GET
    if (parsedRequest.method != "GET") {
        // 400 Bad Request
        SendErrorResponse(clientSocket, clientAddr, 400, pathStr);
    }
    else {
        if (UrlMap.find(pathStr) != UrlMap.end()) {
            // 301 Moved Permanently
            SendErrorResponse(clientSocket, clientAddr, 301, pathStr);
        }
        else {
            string filePath = serverConfig.HOME + pathStr;
            ifstream fileStream(filePath, ios::binary);

            if (!fileStream) {
                // 404 Not Found
                SendErrorResponse(clientSocket, clientAddr, 404, pathStr);
            }
            else {
                // 200 OK
                cout << "Response: " << "200 OK" << endl;
                string fileExtension = pathStr.substr(pathStr.rfind('.'));
                ResponseHeader = "HTTP/1.1 200 OK\r\nContent-type: " + contentTypes[fileExtension] + "\r\n\r\n";
                send(clientSocket, ResponseHeader.c_str(), ResponseHeader.length(), 0);

                while (!fileStream.eof()) {
                    fileStream.read(buffer, sizeof(buffer));
                    int readLen = fileStream.gcount();
                    send(clientSocket, buffer, readLen, 0);
                }

                fileStream.close();
                closesocket(clientSocket);
            }
        }
    }
}


int main() {

    // 读取配置文件
    ServerConfig serverConfig;
    ifstream configFile("./config.txt");
    if (!configFile.is_open()) {
        cerr << "Failed to open config file!\n";
        return 1;
    }
    configFile >> serverConfig.IP >> serverConfig.PORT >> serverConfig.HOME >> serverConfig.WaitCapcity;
    configFile.close();

    // 初始化WinSock
    WSADATA wsaData;
    int nRc = WSAStartup(0x0202, &wsaData);
    if (nRc) {
        cerr << "Winsock startup failed with error!\n";
        return 1;
    }
    if (wsaData.wVersion != 0x0202) {
        cerr << "Winsock version is not correct!\n";
        WSACleanup();
        return 1;
    }
    cout << "Winsock startup OK!\n";

    // 监听socket
    SOCKET srvSocket;

    // 服务器和客户端
    sockaddr_in addr;

    // 会话socket，负责和client进程通信
    SOCKET sessionSocket;

    // ip地址长度
    int addrLen;

    // 创建监听socket
    srvSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (srvSocket == INVALID_SOCKET) {
        cerr << "Create listen socket failed!\n";
        WSACleanup();
        return 1;
    }
    cout << "Create listen socket success!\n";

    // 设置服务器的端口和地址
    addr.sin_family = AF_INET;
    //addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    addr.sin_addr.S_un.S_addr = inet_addr(serverConfig.IP);
    addr.sin_port = htons(serverConfig.PORT);

    // 绑定
    if (bind(srvSocket, (LPSOCKADDR)&addr, sizeof addr) == SOCKET_ERROR) {
        cerr << "Socket bind failed!\n";
        closesocket(srvSocket);
        WSACleanup();
        return 1;
    }
    cout << "Socket bind ok!\n";

    // 监听
    if (listen(srvSocket, serverConfig.WaitCapcity) == SOCKET_ERROR) {
        cerr << "Socket listen failed!\n";
        closesocket(srvSocket);
        WSACleanup();
        return 1;
    }
    cout << "Socket listen ok!\n";

    cout << "HTTP server is running on port " << serverConfig.PORT << " ..." << endl;

    // 响应请求
    while (true) {
        cout << "-----------------------------------------------------------------------------" << endl;
        // 连接请求
        sockaddr_in clientAddr;
        addrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(srvSocket, (LPSOCKADDR)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Accept failed!\n";
            closesocket(srvSocket);
            WSACleanup();
            return 1;
        }

        // 处理HTTP请求
        HandleHttpRequest(clientSocket, serverConfig);
    }

    // 服务器关闭
    closesocket(srvSocket);
    WSACleanup();
    return 0;
}
