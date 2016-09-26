#pragma once

#include "Message.h"
#include "MessageType.h"
#include "Address.h"
#include "Channel.h"

class NetworkMessage {

public:
	Message * msg;
	Channel channel;
	MessageType type;
	Address hopsource;
	Address hopdestination;
	Address source;
	Address destination;

};