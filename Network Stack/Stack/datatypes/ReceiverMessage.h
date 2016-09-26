#pragma once

#include <stdint.h>
#include "Message.h"
#include "Angle.h"

class ReceiverMessage {
public:
	Message * msg;
	uint16_t signalstrength;
	Angle startangle;
	Angle endangle;

	ReceiverMessage(){
		this->msg = new Message(0);
	};
	ReceiverMessage(uint8_t length) {
		this->msg = new Message(length);
	};
	~ReceiverMessage(){
		this->msg->~Message();
		this->msg = nullptr;
	};
};