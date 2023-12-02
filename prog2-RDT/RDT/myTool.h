#pragma once

/*
定义工具函数
*/

#include <deque>
#include <unordered_map>
#include "DataStructure.h"

/*
打印窗口
*/
void printWindowStatusDeque(const char* prompt, const std::deque<std::pair<Packet, bool>>& window, int winLen);
void printWindowStatusHashTable(const char* prompt, const std::unordered_map<unsigned int, Packet> window, unsigned int base, unsigned int winLen);
void printWindowStatusGBN(const char* prompt, const std::deque<Packet>& window, unsigned int base, int winLen);
