#include "NetworkLayer.h"


NetworkLayer::NetworkLayer(DatalinkLayer * datalinklayer, Address ownaddress)
{
	this->dll = datalinklayer;
	this->myaddress = ownaddress;

	this->_continue = true;
	workThread = std::thread(&NetworkLayer::processIncomingData, this);
}


NetworkLayer::~NetworkLayer()
{
	this->_continue = false;
	workThread.join();
}

NetworkMessage * NetworkLayer::read(void) {
	if (messagequeue.isEmpty()) {
		return 0;
	}
	
	return messagequeue.pop();
}

WriteResult NetworkLayer::write(Address destination, Message * msg) {
	return write(destination, this->myaddress, msg);
}

WriteResult NetworkLayer::write(Address destination, Address source, Message * msg) {
	// Resolve the neighbor with the best route to the final destination.
	Address nexthop = this->router.getNextAddressAlongRoute(this->myaddress, destination);

	// Assemble a new message.
	Message * newmsg = new Message(msg->length + 4);
	newmsg->data[0] = destination >> 8;
	newmsg->data[1] = destination & 0xff;
	newmsg->data[2] = source >> 8;
	newmsg->data[3] = source & 0xff;

	// Copy the data.
	for (int i = 0; i < (newmsg->length); i++)
		newmsg->data[i + 4] = msg->data[i];

	// Send it.
	dll->write(nexthop, newmsg);

	delete newmsg;

	return WRITE_SUCCESS;
}

bool NetworkLayer::registerNeighbor(Address address, Angle startangle, Angle endangle) {
	router.addNeighbor(this->myaddress, address);
	return dll->registerNeighbor(address, startangle, endangle);
}

bool NetworkLayer::registerNode(Address parent, Address neighbor) {
	
	return this->router.addNeighbor(parent, neighbor);
}

uint16_t NetworkLayer::discoverNetwork(Angle step) {
	uint16_t count = 0;

	if ((step < 1) | (step > 360))
		step = 90;

	Queue<Address> * discoveredneighborlist = dll->discoverNeighbors(step);

	while (!discoveredneighborlist->isEmpty()) {
		if (registerNode(this->myaddress, discoveredneighborlist->pop()))
			count++;	// Only count real new neighbors.
	}

	delete discoveredneighborlist;

	return count;
}

uint16_t NetworkLayer::discoverNetwork(Angle startangle, Angle endangle) {
	uint16_t count = 0;

	Queue<Address> * discoveredneighborlist = dll->discoverNeighbors(startangle, endangle);

	while (!discoveredneighborlist->isEmpty()) {
		if(registerNode(this->myaddress, discoveredneighborlist->pop()))
			count++;	// Only count real new neighbors.
	}

	delete discoveredneighborlist;

	return count;
}


void NetworkLayer::processIncomingData(void) {

	while (this->_continue) {
		NetworkMessage * msg = dll->read();	// Read data.

		if (msg)
			processIncomingMessage(msg);
	}
}

void NetworkLayer::processIncomingMessage(NetworkMessage * netmsg) {
	NetworkMessage * newmessage;

	if (netmsg->msg->length < 5) {
		delete netmsg;
		return;
	}

	switch (netmsg->type){
		case MESSAGENETWORKINFOEXCHANGE:

			break;
		case MESSAGENETWORKINFOREQ:

			break;
		case MESSAGEDATA:
			newmessage = new NetworkMessage;
			newmessage->hopdestination = netmsg->hopdestination;
			newmessage->hopsource = netmsg->hopsource;
			newmessage->type = netmsg->type;
			newmessage->channel = netmsg->channel;

			// Parse the network destination and source.
			newmessage->destination = (Address)(netmsg->msg->data[0] << 8) | (netmsg->msg->data[1]);
			newmessage->source = (Address)(netmsg->msg->data[2] << 8) | (netmsg->msg->data[3]);

			newmessage->msg = new Message(netmsg->msg->length - 4);	// Create a new msg;

			// Copy the data.
			for (int i = 0; i < newmessage->msg->length; i++) {
				newmessage->msg->data[i] = netmsg->msg->data[i + 4];
			}

			// If it is for us, store it. Else resend it.
			if (newmessage->destination == this->myaddress || newmessage->destination == BROADCAST_ADDRESS) {
				messagequeue.push(newmessage);	// Put it in the queue.
			}
			else {
				// Retransmit it.
				write(newmessage->destination, newmessage->source, newmessage->msg);
				delete newmessage;
			}

			// Delete the old message.
			delete netmsg;

			break;
		default:

			// Delete the old message.
			delete netmsg;

			break;
	}

}