#include "DatalinkLayer.h"

DatalinkLayer::DatalinkLayer(PhysicalLayer * phy, Address address)
{
	if (phy == NULL) {
		throw std::invalid_argument("Null pointer to the PhysicalLayer.");
		exit(0);
	}

	startangle[CHANNEL1] = 315;
	startangle[CHANNEL2] = 135;
	startangle[CHANNEL3] = 225;
	startangle[CHANNEL4] = 45;
	endangle[CHANNEL1] = 45;
	endangle[CHANNEL2] = 225;
	endangle[CHANNEL3] = 315;
	endangle[CHANNEL4] = 135;

	this->physical = phy;
	this->myaddress = address;

	this->_continue = true;
	readThread = std::thread(&DatalinkLayer::processIncomingData, this);
}


DatalinkLayer::~DatalinkLayer()
{
	this->_continue = false;
	readThread.join();
}

WriteResult DatalinkLayer::write(Address destination, Message * message)
{
	Message payload;

	/************************************************/
	// Data preparation.
	// Assemble a Datalink Protocol Data unit with the neighbor destination address encoded in the payload.
	payload.length  = message->length + 5;
	payload.data    = new uint8_t[payload.length];

	payload.data[0] = (uint8_t)(destination >> 8);		// Encode the destination address in the payload.
	payload.data[1] = (uint8_t)(destination & 0xff);
	payload.data[2] = (uint8_t)(myaddress >> 8);		// Encode my address in the payload.
	payload.data[3] = (uint8_t)(myaddress & 0xff);
	payload.data[4] = (uint8_t) MESSAGEDATA;

	// Copy the elements to the payload Message.
	for (uint16_t i = 0; i < message->length; i++)
		payload.data[5 + i] = message->data[i];

	// Send it
	WriteResult result =  send(destination, &payload);

	return result;
}

WriteResult DatalinkLayer::send(Address destination, Message * message)
{
	Angle startangle = 0;
	Angle endangle = 359;

	/************************************************/
	// Data preparation.

	// Check if we have a broadcast. If not, we have already set the angles.
	if (destination != BROADCAST_ADDRESS) {
		// Resolve the angle of the neighbor.
		vector<Neighbor>::iterator it = find_if(this->neighbors.begin(), this->neighbors.end(), findNeighbor(destination));
		// Check if the neighbor exists in the database.
		if (it == this->neighbors.end()) {
			return WRITE_FAIL;
		}

		startangle = it->startangle;
		endangle = it->endangle;
	}

	return send(startangle, endangle, message);
}

WriteResult DatalinkLayer::send(Angle startangle, Angle endangle, Message * message)
{
	/************************************************/
	// Medium Access and Transmission Control
	// Run access control within a loop to accomodate multiple attempts.
	for (uint16_t retrycounter = 0; retrycounter < MAC_BACKOFF_RETRYLIMIT; retrycounter++) {

		// Get the medium state in the transmission direction.
		bool mediumready = true;
		if (((startangle >= 45) & (startangle < 135)) | ((endangle >= 45) & (endangle < 135))) {
			mediumready &= (this->physical->getMediumState(CHANNEL4) == MEDIUM_IDLE);	// Check the direction of photodiode 4.
		}
		if (((startangle >= 135) & (startangle < 225)) | ((endangle >= 135) & (endangle < 225))) {
			mediumready &= (this->physical->getMediumState(CHANNEL2) == MEDIUM_IDLE);	// Check the direction of photodiode 2.
		}
		if (((startangle >= 225) & (startangle < 315)) | ((endangle >= 225) & (endangle < 315))) {
			mediumready &= (this->physical->getMediumState(CHANNEL3) == MEDIUM_IDLE);	// Check the direction of photodiode 3.
		}
		if (((startangle < 45) | (startangle >= 315)) | ((endangle < 45) | (endangle >= 315))) {
			mediumready &= (this->physical->getMediumState(CHANNEL1) == MEDIUM_IDLE);	// Check the direction of photodiode 1.
		}

		// Check if the medium is idle.
		if (mediumready) {
			// Yes it is, send the package.
			WriteResult result = physical->write(message, startangle, endangle);

			// Return the result.
			return result;
		}

		// If the medium is not idle, back off and retry..
		Sleep(calculateBackoffDelay(retrycounter));
	}

	// The retry limit has been reached, return WriteFailed.
	return WRITE_FAIL;
}

NetworkMessage * DatalinkLayer::read(void)
{
	return messagequeue.pop();
}

Queue<Address>* DatalinkLayer::discoverNeighbors(Angle anglestep)
{
	Queue<Address> * result = new Queue<Address>();
	Queue<Address> * neighborlist;

	for (Angle startangle = 0; startangle <= (360 - anglestep); startangle = startangle + anglestep) {
		// For each input channel.
		neighborlist = discoverNeighbors(startangle, (startangle + anglestep));
		while (!neighborlist->isEmpty()) {
			result->push(neighborlist->pop());
		}
		delete neighborlist;
	}

	return result;
}

/**
* Transmit a PING packet in a certain direction in order to find neighbors.
* @param startangle,endangle The direction in which the PING should be send.
* @return WriteResult.
*/
Queue<Address> * DatalinkLayer::discoverNeighbors(Angle startangle, Angle endangle)
{
	Queue<Address> * result = new Queue<Address>();

	Message * msg = createPingRequestMessage(BROADCAST_ADDRESS);	// Create a discovery package.

	if (send(startangle, endangle, msg) != WRITE_SUCCESS) {			// Send the ping.
		return result;
	}

	Sleep(500); // Allow processing.

	delete msg;

	while (!pingreplyqueue.isEmpty()) {	// Check if we have found something.
		Neighbor * neighbor = pingreplyqueue.pop();	// Yes.

		// Enhance discovery precision.
/*		if ((endangle - startangle) < (neighbor->endangle - neighbor->startangle)) {
			registerNeighbor(neighbor->address, startangle, endangle);	// Register the neighbor, or update its position.
		}
		else {
			registerNeighbor(neighbor->address, neighbor->startangle, neighbor->endangle);	// Register the neighbor, or update its position.
		}*/

		// Increase precision.
		if (startangle > neighbor->startangle) {
			neighbor->startangle = startangle;
		}
		if (endangle < neighbor->endangle) {
			neighbor->endangle = endangle;
		}
		registerNeighbor(neighbor->address, neighbor->startangle, neighbor->endangle);	// Register the neighbor, or update its position.
		
		result->push(neighbor->address);
	}

	return result;
}

bool DatalinkLayer::registerNeighbor(Address address, Angle startangle, Angle endangle)
{
	// Check if we already know the neighbor.
	vector<Neighbor>::iterator it = find_if(this->neighbors.begin(), this->neighbors.end(), findNeighbor(address));
	if (it == this->neighbors.end()) {
		// No, add it to our list.
		this->neighbors.push_back(Neighbor(address, startangle, endangle));
		return true;
	}
	else {
		// Increase precision.
		//if (startangle > it->startangle) {
			it->startangle = startangle;
		//}
		//if (endangle < it->endangle) {
			it->endangle = endangle;
		//}
	}
	
	return false;
}

/**
* This function calculates the backoff delay in milliseconds.
* @param attemptcount The number of times the software has backed off.
* @return uint16_t The delay in milliseconds.
*/
inline uint16_t DatalinkLayer::calculateBackoffDelay(uint16_t attemptcount) {
	return 300 * attemptcount;
}

void DatalinkLayer::processIncomingData(void) {

	while (this->_continue) {
		for (uint8_t ch = CHANNEL1; ch < NUM_CHANNELS; ch++) {

			// Check if we have sufficient bytes available.
			Message * msg = physical->read((Channel) ch);
			if (msg) {
				processIncomingMessage(msg, (Channel) ch);
			}
		}
	}
}

void DatalinkLayer::processIncomingMessage(Message * msg, Channel ch) {
	NetworkMessage result;
	NetworkMessage * newmessage;

	if (!msg)
		return;

	if (msg->length < 5) {
		delete msg;
		return;
	}

	// Set the channel.
	result.channel = ch;

	// Read the hop destination.
	result.hopdestination = ((msg->data[0] << 8) | (msg->data[1]));

	// Check addresses.
	if ((result.hopdestination != this->myaddress) && (result.hopdestination != BROADCAST_ADDRESS)) {
		delete msg;
		return;
	}

	// Read the hop source.
	result.hopsource = ((msg->data[2] << 8) | (msg->data[3]));

	// Read the message type.
	result.type = (MessageType) msg->data[4];

	registerNeighbor(result.hopsource, startangle[ch], endangle[ch]);
	//TODO improve registration resolution.

	switch (result.type) {
	case MESSAGEREQPING:
			send(result.hopsource, createPingAnswerMessage(result.hopsource));
			return;
		break;

	case MESSAGEANSPING:
			pingreplyqueue.push(new Neighbor(result.hopsource, startangle[ch], endangle[ch]));
			// TODO: network information exchange.
			//write(result->hopsource, createNetworkInformationMessage(result->hopsource);
			//return;
		break;

	case MESSAGENETWORKINFOREQ:
			//write(result.hopsource, createNetworkInformationMessage(result.hopsource));
		break;

	case MESSAGEDATA:
			newmessage = new NetworkMessage();
			newmessage->hopsource = result.hopsource;
			newmessage->hopdestination = result.hopdestination;
			newmessage->channel = result.channel;
			newmessage->type = result.type;
			newmessage->msg = new Message(msg->length - 5);

			// Copy the data.
			for (int i = 0; i < newmessage->msg->length; i++) {
				newmessage->msg->data[i] = msg->data[i + 5];
			}

			// Store the data.
			messagequeue.push(newmessage);
		break;
//	case MESSAGENETWORKINFOEXCHANGE:
	default:
		// Invalid message.
		delete msg;
	}
}

Message * DatalinkLayer::createPingRequestMessage(Address destination) {
	Message * result = new Message(6);

	result->data[0] = (uint8_t)((destination >> 8) & 0xff);
	result->data[1] = (uint8_t)((destination)& 0xff);
	result->data[2] = (uint8_t)((this->myaddress >> 8) & 0xff);
	result->data[3] = (uint8_t)((this->myaddress) & 0xff);
	result->data[4] = MESSAGEREQPING;
	result->data[5] = 0x00;

	return result;
}

Message * DatalinkLayer::createPingAnswerMessage(Address destination) {
	Message * result = new Message(6);

	result->data[0] = (uint8_t)((destination >> 8) & 0xff);
	result->data[1] = (uint8_t)((destination)& 0xff);
	result->data[2] = (uint8_t)((this->myaddress >> 8) & 0xff);
	result->data[3] = (uint8_t)((this->myaddress) & 0xff);
	result->data[4] = MESSAGEANSPING;
	result->data[5] = 0x00;

	return result;
}

Message * DatalinkLayer::createNetworkInformationMessage(Address destination) {
	Message * result = new Message(6);
	/*
	result->data[0] = (uint8_t)((destination >> 8) & 0xff);
	result->data[1] = (uint8_t)((destination)& 0xff);
	result->data[2] = (uint8_t)((this->myaddress >> 8) & 0xff);
	result->data[3] = (uint8_t)((this->myaddress) & 0xff);
	result->data[4] = MESSAGEANSPING;
	result->data[5] = 0x00;*/

	return result;
}