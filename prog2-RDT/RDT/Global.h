#pragma once

#include "Tool.h"
#include "NetworkService.h"


extern Tool *pUtils;						//指向唯一的工具类实例，只在main函数结束前delete
extern NetworkService *pns;				//指向唯一的模拟网络环境类实例，只在main函数结束前delete

// 定义ANSI颜色代码
#define RESET   "\033[0m"
#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"