#pragma once

#include "RdtReceiver.h"

class GoBackNRdtReceiver :public RdtReceiver
{
public:
	GoBackNRdtReceiver();
	virtual ~GoBackNRdtReceiver();

private:
	int expectedSeqNum;
	Packet lastAckPkt;
	int seqLen;

public:
	void receive(const Packet& packet);
};