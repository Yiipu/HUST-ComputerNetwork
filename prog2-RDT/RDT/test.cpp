// test.cpp : 定义控制台应用程序的入口点。

#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "StopWaitRdtSender.h"
#include "StopWaitRdtReceiver.h"
#include "GoBackNRdtSender.h"
#include "GoBackNRdtReceiver.h"
#include "SelectiveRepeatRdtSender.h"
#include "SelectiveRepeatRdtReceiver.h"
#include "TransmissionControlRdtSender.h"
#include "TransmissionControlRdtReceiver.h"

#include <map>

// 使用自动测试程序时，取消下面行的注释，并将 choice 定义为要检查的协议名（SW/GNB/SR）
// #define choice "TCP"
#define FILE_PATH_INPUT "input.txt"
#define FILE_PATH_OUTPUT "output.txt"
#define RUN_MODE 1 //控制提示信息。1是安静模式，0是详细模式

int main(int argc, char* argv[])
{
	std::map<std::string, std::pair<RdtSender*, RdtReceiver*>> protocols = {
		{"SW", {new StopWaitRdtSender(), new StopWaitRdtReceiver()}},
		{"GBN", {new GoBackNRdtSender(), new GoBackNRdtReceiver()}},
		{"SR", {new SelectiveRepeatRdtSender(), new SelectiveRepeatRdtReceiver()}},
		{"TCP", {new TransmissionControlRdtSender(), new TransmissionControlRdtReceiver()}}
	};

	RdtSender* ps = nullptr;
	RdtReceiver* pr = nullptr;

	#ifndef choice
	std::string choice;
	do {
		std::cout << "Choose a protocol (SW for Stop-and-Wait, GBN for Go-Back-N, SR for Selective-Repeat, TCP for Transmission-Control): ";
		std::cin >> choice;
	} while (protocols.find(choice) == protocols.end());
	#endif

	ps = protocols[choice].first;
	pr = protocols[choice].second;
	
	pns->setRunMode(RUN_MODE);
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile(FILE_PATH_INPUT);
	pns->setOutputFile(FILE_PATH_OUTPUT);

	pns->start();

	// Cleanup memory
	for (auto& pair : protocols) {
		delete pair.second.first;
		delete pair.second.second;
	}

	delete pUtils;									//指向唯一的工具类实例，只在main函数结束前delete
	delete pns;										//指向唯一的模拟网络环境类实例，只在main函数结束前delete
	
	return 0;
}

