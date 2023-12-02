#pragma once

#include "RdtSender.h"
#include<queue>

class TransmissionControlRdtSender :public RdtSender
{
public:
	TransmissionControlRdtSender();
	virtual ~TransmissionControlRdtSender();

private:
	unsigned int base;								// 基序号
	unsigned int nextSeqNum;						// 下一个序号
	std::pair<unsigned int, short> lashAckFreq;		// 上一个ACK及其出现次数
	int seqLen;										// 序号空间大小
	int winLen;										// 窗口长度
	std::deque<Packet> window;						// 窗口队列
	bool waitingState = window.size() == winLen;	// 等待状态定义为窗口满

public:
	bool getWaitingState();
	bool send(const Message& message);				//发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void receive(const Packet& ackPkt);				//接受确认Ack，将被NetworkServiceSimulator调用	
	void timeoutHandler(int seqNum);
};