# 基于 WinSocket2 的服务器源码

## 配置

运行之前自行创建两个文本文档 `config.txt` 和 `log.txt`

`config.txt` 示例配置：

```text
127.0.0.1
3000
../resources
32
```

配置的含义是：

1. IP
2. PORT
3. 资源文件夹
4. WaitCapcity

## 资源

资源文件夹内的文件都可以通过链接 `IP:PORT/` 获取。
