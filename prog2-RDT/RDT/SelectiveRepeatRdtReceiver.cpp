#include "stdafx.h"
#include "Global.h"
#include "SelectiveRepeatRdtReceiver.h"

#define getSeqPos(seqNum) (seqNum - this->base + this->seqLen) % this->seqLen

SelectiveRepeatRdtReceiver::SelectiveRepeatRdtReceiver():base(1), winLen(8), seqLen(16)
{
	ackPkt.acknum = 0; //初始状态下，上次发送的确认包的确认序号为-1，使得当第一个接受的数据包出错时该确认报文的确认号为-1
	ackPkt.checksum = 0;
	ackPkt.seqnum = -1;	//忽略该字段
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		ackPkt.payload[i] = '.';
	}
	ackPkt.checksum = pUtils->calculateCheckSum(ackPkt);
}

SelectiveRepeatRdtReceiver::~SelectiveRepeatRdtReceiver()
{
	window.clear();
}

void SelectiveRepeatRdtReceiver::receive(const Packet& packet)
{
	int checkSum = pUtils->calculateCheckSum(packet);

	//如果校验和正确，同时收到报文的序号在窗口内
	if (checkSum == packet.checksum 
		&& getSeqPos(packet.seqnum) <= this->winLen - 1)
	{
		pUtils->printPacket("接收方正确收到发送方的报文", packet);

		// 发送ACK
		ackPkt.acknum = packet.seqnum;
		ackPkt.checksum = pUtils->calculateCheckSum(ackPkt);
		pUtils->printPacket("接收方发送确认报文", ackPkt);
		pns->sendToNetworkLayer(SENDER, ackPkt);

		// 如果分组是以前没收到过的，将其缓存
		if (window.find(packet.seqnum) == window.end()) {
			window.insert(std::make_pair(packet.seqnum, packet));
			printWindowStatusHashTable("SR接收方窗口(新报文)：", this->window, this->base, this->winLen);
		}
		// 如果收到基分组
		if (packet.seqnum == this->base)
		{
			printWindowStatusHashTable("SR接收方窗口(滑动前)：", this->window, this->base, this->winLen);
			// 移动窗口
			while (window.find(this->base) != window.end())
			{
				auto iter = window.find(this->base);

				// 取出Message，向上递交给应用层
				Message msg;
				memcpy(msg.data, iter->second.payload, sizeof(packet.payload));
				pns->delivertoAppLayer(RECEIVER, msg);

				window.erase(iter);
				this->base = (this->base + 1)%this->seqLen;
			}
			printWindowStatusHashTable("SR接收方窗口(滑动后)：", this->window, this->base, this->winLen);
		}
		
	}
	else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
		}
		// TODO
		//else if (packet.seqnum < this->base
		//	&& (packet.seqnum >= this->base - this->winLen || this->base < this->winLen)) {
		else{
			pUtils->printPacket("接收方收到确认过的报文，再次发送确认报文", packet);
			// 发送ACK
			ackPkt.acknum = packet.seqnum;
			ackPkt.checksum = pUtils->calculateCheckSum(ackPkt);
			pns->sendToNetworkLayer(SENDER, ackPkt);
		}
		//else {
		//	pUtils->printPacket("接收方收到序号错误的报文，忽略", packet);
		//}
	}
}
