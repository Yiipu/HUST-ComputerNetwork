﻿#include "stdafx.h"
#include "Global.h"
#include "TransmissionControlRdtReceiver.h"

TransmissionControlRdtReceiver::TransmissionControlRdtReceiver() :expectedSeqNum(1), seqLen(16)
{
	lastAckPkt.acknum = 1; 
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//忽略该字段
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}

TransmissionControlRdtReceiver::~TransmissionControlRdtReceiver()
{
}

void TransmissionControlRdtReceiver::receive(const Packet& packet)
{
	int checkSum = pUtils->calculateCheckSum(packet);

	// 如果校验和正确，同时收到报文的序号等于接收方期待收到的报文序号一致
	if (checkSum == packet.checksum && this->expectedSeqNum == packet.seqnum) {
		pUtils->printPacket("接收方正确收到发送方的报文", packet);

		// 与 GBN 接收方不同之处：提前更新 expectedSeqNum
		this->expectedSeqNum = (this->expectedSeqNum + 1) % this->seqLen;

		// 取出Message，向上递交给应用层
		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, msg);

		lastAckPkt.acknum = this->expectedSeqNum;
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("接收方发送确认报文", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
	}
	else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
		}
		else {
			pUtils->printPacket("接收方没有正确收到发送方的报文,报文序号不对", packet);
		}
		pUtils->printPacket("接收方重新发送上次的确认报文", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送上次的确认报文
	}
}