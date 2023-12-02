#include "stdafx.h"
#include "Global.h"
#include "SelectiveRepeatRdtSender.h"

#define getSeqPos(seqNum) (seqNum - this->base + this->seqLen) % this->seqLen

SelectiveRepeatRdtSender::SelectiveRepeatRdtSender() :base(1), nextSeqNum(1), winLen(8), seqLen(16)
{
}

SelectiveRepeatRdtSender::~SelectiveRepeatRdtSender()
{
	window.clear();
}

bool SelectiveRepeatRdtSender::getWaitingState()
{
	return waitingState;
}

bool SelectiveRepeatRdtSender::send(const Message& message)
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

	// 压入窗口队列，标记为未收到确认
	this->window.push_back({ packet, false });
	printWindowStatusDeque("发送方窗口(新报文)：", this->window, this->winLen);

	// 启动定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, packet.seqnum);

	// 待发分组序号递增
	this->nextSeqNum = (this->nextSeqNum + 1) % this->seqLen;

	return true;
}

void SelectiveRepeatRdtSender::receive(const Packet& ackPkt)
{
	// 计算校验和
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	if (checkSum == ackPkt.checksum && 
		((this->nextSeqNum >= this->base && ackPkt.acknum >= this->base && ackPkt.acknum < this->nextSeqNum)
		|| (this->nextSeqNum < this->base && (ackPkt.acknum >= this->base || ackPkt.acknum < this->nextSeqNum))))
		// 若校验正确，且序号在范围内，作如下处理
	{
		pUtils->printPacket("发送方正确收到确认", ackPkt);
		pns->stopTimer(SENDER, ackPkt.acknum); // 如果计时器不允许重复停止，需要修改这里

		// 标记为收到确认
		window.at(getSeqPos(ackPkt.acknum)).second = true;

		// 如果收到基号确认
		if (ackPkt.acknum == this->base)
		{
			printWindowStatusDeque("SR发送方窗口(滑动前)：", this->window, this->winLen);
			// 移动窗口
			while (!window.empty() && window.front().second)
			{
				window.pop_front();
				this->base = (this->base + 1) % this->seqLen;
			}
			printWindowStatusDeque("SR发送方窗口(滑动后)：", this->window, this->winLen);
		}
		else if (checkSum != ackPkt.checksum)
			pUtils->printPacket("发送方没有正确收到该报文确认,数据校验错误", ackPkt);
		else
			pUtils->printPacket("发送方已正确收到该报文确认,报文序号不在范围内", ackPkt);
	}
}

void SelectiveRepeatRdtSender::timeoutHandler(int seqNum)
{
	pns->stopTimer(SENDER, seqNum);
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);
	pUtils->printPacket("发送方计时器超时，重发窗口报文", window.at(getSeqPos(seqNum)).first);
	pns->sendToNetworkLayer(RECEIVER, window.at(getSeqPos(seqNum)).first);
}
