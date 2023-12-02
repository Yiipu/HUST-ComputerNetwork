#pragma once

#include "RdtReceiver.h"
#include <unordered_map>

class SelectiveRepeatRdtReceiver :public RdtReceiver
{
public:
	SelectiveRepeatRdtReceiver();
	virtual ~SelectiveRepeatRdtReceiver();

private:
	unsigned int base;									// 基序号
	int seqLen;
	int winLen;											// 窗口长度
	std::unordered_map<unsigned int, Packet> window;	// 哈希表，将序列号映射到数据帧
	Packet ackPkt;

public:
	void receive(const Packet& packet);
};