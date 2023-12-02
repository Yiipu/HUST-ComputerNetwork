#include "stdafx.h"
#include "Global.h"
#include "GoBackNRdtSender.h"

GoBackNRdtSender::GoBackNRdtSender():base(1), nextSeqNum(1), winLen(8), seqLen(16)
{
}

GoBackNRdtSender::~GoBackNRdtSender()
{
	window.clear();
}

bool GoBackNRdtSender::getWaitingState()
{
	return waitingState;
}

bool GoBackNRdtSender::send(const Message& message)
{
	if (this->waitingState)
	// 若窗口满，则拒绝发送
		return false;

	// 打包阶段
	Packet packet;
	packet.acknum = -1;	// 忽略该字段
	packet.seqnum = this->nextSeqNum;
	memcpy(packet.payload, message.data, sizeof(message.data));
	packet.checksum = pUtils->calculateCheckSum(packet);
	pUtils->printPacket("发送方将发送报文", packet);

	// 压入窗口队列
	this->window.push_back(packet);
	
	if (this->base == this->nextSeqNum)
		// 若不存在已发送但还未被确认的包 ，则启动新的计时器。
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);

	// 发送到网络层
	pns->sendToNetworkLayer(RECEIVER, packet);

	// 待发分组序号递增
	this->nextSeqNum = (this->nextSeqNum + 1) % this->seqLen;

	return true;
}

void GoBackNRdtSender::receive(const Packet& ackPkt)
{
	// 计算校验和
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	if (checkSum == ackPkt.checksum && 
		((this->nextSeqNum >= this->base && ackPkt.acknum >= this->base && ackPkt.acknum < this->nextSeqNum)
			||(this->nextSeqNum < this->base && (ackPkt.acknum >= this->base || ackPkt.acknum < this->nextSeqNum))))
		// 若校验正确，且序号在范围内，作如下处理
	{
		pUtils->printPacket("发送方正确收到确认", ackPkt);
		pns->stopTimer(SENDER, this->base); // 如果计时器不允许重复停止，需要修改这里

		printWindowStatusGBN("GBN发送方窗口：", this->window, this->base, this->winLen);
		// 移动窗口，接收方遵循累积ACK原则，所以 base 到 acknum 之间的分组一定到达
		while (this->base != (ackPkt.acknum + 1) % this->seqLen)
		{
			window.pop_front();
			this->base = (this->base + 1) % this->seqLen;
		}
		printWindowStatusGBN("GBN发送方窗口：", this->window, this->base, this->winLen);

		// 还有已发送但还未被确认的包,启动新计时器
		if (!(this->base == this->nextSeqNum))
			pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);
	}
	// 校验错误或报文序号不在范围内
	else if (checkSum != ackPkt.checksum)
		pUtils->printPacket("发送方没有正确收到该报文确认,数据校验错误", ackPkt);
	else
		pUtils->printPacket("发送方已正确收到该报文确认,报文序号不在范围内", ackPkt);
}

void GoBackNRdtSender::timeoutHandler(int seqNum)
{
	pns->stopTimer(SENDER, seqNum);
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);
	for (auto i = window.begin(); i != window.end(); i++) {
		pUtils->printPacket("发送方计时器超时，重发窗口报文", *i);
		pns->sendToNetworkLayer(RECEIVER, *i);
	}
}