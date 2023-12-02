#pragma once

#include "RdtReceiver.h"

class TransmissionControlRdtReceiver :public RdtReceiver
{
public:
	TransmissionControlRdtReceiver();
	virtual ~TransmissionControlRdtReceiver();

private:
	int expectedSeqNum;
	Packet lastAckPkt;
	int seqLen;

public:
	void receive(const Packet& packet);
};