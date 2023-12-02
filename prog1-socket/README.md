# 基于 WinSocket2 的服务器源码

## 配置

运行之前在根目录自行创建两个文件 `config.txt` 和 `log.txt`

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

资源文件夹内的文件都可以通过链接 `IP:PORT/` 获取。如果PORT为80，那么通过 `IP:PORT` 可以直接获取 `index.html` 。
