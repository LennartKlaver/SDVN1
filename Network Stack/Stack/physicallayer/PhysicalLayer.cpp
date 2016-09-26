#include "PhysicalLayer.h"

#define UART_CONTROL_SETDIRECTION 0x81

uint8_t LUT_START[41] = {
	0xFF,
	0xFE, 0xFE, 0xFC, 0xFC, 0xF8, 0xF8, 0xF0, 0xF0,
	0xE0, 0xE0, 0xC0, 0xC0, 0x80, 0x80, 0xFF, 0xFF,
	0xFE, 0xFE, 0xFC, 0xFC, 0xF8, 0xF8, 0xF0, 0xF0,
	0xE0, 0xE0, 0xC0, 0xC0, 0x80, 0x80, 0x0F, 0x0F,
	0x0E, 0x0E, 0x0C, 0x0C, 0x08, 0x08,
	0x00, 0x00
};

uint8_t LUT_END[41] = {
	0x01,
	0x03, 0x03, 0x07, 0x07, 0x0F, 0x0F, 0x1F, 0x1F,
	0x3F, 0x3F, 0x7F, 0x7F, 0xFF, 0xFF, 0x01, 0x01,
	0x03, 0x03, 0x07, 0x07, 0x0F, 0x0F, 0x1F, 0x1F,
	0x3F, 0x3F, 0x7F, 0x7F, 0xFF, 0xFF, 0x01, 0x01,
	0x03, 0x03, 0x07, 0x07, 0x0F, 0x0F,	0xff, 0x80
};

PhysicalLayer::PhysicalLayer(Serial * port)
{
	serialport = port;

	mediumstate[CHANNEL1] = MEDIUM_IDLE;
	mediumstate[CHANNEL2] = MEDIUM_IDLE;
	mediumstate[CHANNEL3] = MEDIUM_IDLE;
	mediumstate[CHANNEL4] = MEDIUM_IDLE;

	this->_continue = true;
	readThread = std::thread(&PhysicalLayer::processSerialData, this);
} 


PhysicalLayer::~PhysicalLayer()
{
	this->_continue = false;
	readThread.join();
}

WriteResult PhysicalLayer::write(Message * msg, Angle startangle, Angle endangle) {
	uint8_t LED8_1, LED16_9, LED20_17;

	// Get the bytes to set the LED direction.
	convertAngleToLEDBytes(startangle, endangle, &LED8_1, &LED16_9, &LED20_17);
	
	// Compose the frame.
	// First get the number of occurences of special characters.
	int charactercount = msg->length + 5;	// Charactercount is the message length + 2 for start and end of frame characters.
	for (int i = 0; i < msg->length; i++) {
		if ((msg->data[i] == 0xC0) || (msg->data[i] == 0xDB))
			charactercount++;	// Each special character will be represented by 2 bytes, so add one per case.
	}
	// Then create a message.
	uint8_t * data = new uint8_t[charactercount];
	// And compose it.
	data[0] = 0xC0;
	data[1] = LED8_1;	// Set the transmission direction.
	data[2] = LED16_9;
	data[3] = LED20_17;

	// Add the data to the message, using the Serial Line Internet Protocol (SLIP) algorithm for frame flagging.
	// This means using 0xC0 as start-of-frame and end-of-frame character, and special characters to replace 0xC0 in 
	// data to avoid invalid frame flagging.
	// http://www.tcpipguide.com/free/t_SerialLineInternetProtocolSLIP-2.htm
	int index = 4;
	for (int i = 0; i < msg->length; i++) {
		switch (msg->data[i]) {
			case 0xC0:
				data[index++] = 0xDB;
				data[index++] = 0xDC;
				break;
			case 0xDB:
				data[index++] = 0xDB;
				data[index++] = 0xDD;
				break;
			default:
				data[index++] = msg->data[i];
		}
	}

	data[index] = 0xC0;		// Close the frame.

	// Write the data to the board.
	WriteResult result;
	if (!serialport->write(data, charactercount)) {
		result = WRITE_FAIL;
	}
	else {
		result = WRITE_SUCCESS;
	}

	delete data;	// Free the memory.

	return result;
}

Message * PhysicalLayer::read(Channel channel) {

	if (channel > 3)		// Validate argument input.
		return NULL;

	if (framequeue[channel].isEmpty())	// Check if a frame is available.
		return NULL;

	return framequeue[channel].pop();
}

void PhysicalLayer::convertAngleToLEDBytes(Angle startangle, Angle endangle, uint8_t * LED8_1, uint8_t * LED16_9, uint8_t * LED20_17) {
	uint8_t angle;

	uint8_t start8_1	= 0;
	uint8_t start16_9	= 0;
	uint8_t start20_17	= 0;
	uint8_t end8_1		= 0;
	uint8_t end16_9		= 0;
	uint8_t end20_17	= 0;

	// Determine start angle.
	angle = (uint8_t)(startangle / 9);
	if ((angle < 15)) {
		start8_1 = LUT_START[angle];
		start16_9 = 0xff;
		start20_17 = 0xff;
	}
	else if (angle < 31) {
		start8_1 = 0;
		start16_9 = LUT_START[angle];
		start20_17 = 0xff;
	}
	else {
		start8_1 = 0;
		start16_9 = 0;
		start20_17 = LUT_START[angle];
	}

	// Determine end angle.
	angle = (uint8_t)(endangle / 9);
	if (angle < 15) {
		end8_1 = LUT_END[angle];
		end16_9 = 0;
		end20_17 = 0;
	}
	else if (angle < 31) {
		end8_1 = 0xff;
		end16_9 = LUT_END[angle];
		end20_17 = 0;
	}
	else {
		end8_1 = 0xff;
		end16_9 = 0xff;
		end20_17 = LUT_END[angle];
	}

	// Combine the leds that are selected based on the start angle with those from the end angle.
	// Of course we need to take into account the relative position of the start and end angle.
	if (startangle < endangle) {
		(*LED8_1)   =  start8_1   & end8_1;
		(*LED16_9)  =  start16_9  & end16_9;
		(*LED20_17) = (start20_17 & end20_17) & 0x0f;
	}
	else {
		(*LED8_1)   =  start8_1   | end8_1;
		(*LED16_9)  =  start16_9  | end16_9;
		(*LED20_17) = (start20_17 | end20_17) & 0x0f;
	}
}

/**
 * Read the UART queue and put any incoming data into the right queue.
 */
void PhysicalLayer::processSerialData(void) {
	Channel currentchannel = CHANNEL1;
	uint8_t nextstate = 0;
	uint8_t state = 0;

	while (this->_continue) {		// Loop until the program flags a halt.
		uint8_t buf;

		if (serialport->read(&buf, 1) == 1) {		// Do we have data?
			// Yes.
			// Data from the board is split into two bytes..
			// .. The first byte indicates which channel..
			// .. and the second byte is the actual data.
			if (state == 0) {		// CHANNEL
				mediumstate[CHANNEL1] = ((buf >> 4) & 1) ? MEDIUM_BUSY : MEDIUM_IDLE;
				mediumstate[CHANNEL2] = ((buf >> 5) & 1) ? MEDIUM_BUSY : MEDIUM_IDLE;
				mediumstate[CHANNEL3] = ((buf >> 6) & 1) ? MEDIUM_BUSY : MEDIUM_IDLE;
				mediumstate[CHANNEL4] = ((buf >> 7) & 1) ? MEDIUM_BUSY : MEDIUM_IDLE;
				if ((buf & 0x0f) < 4) {		// Validate the channel number.
					currentchannel = (Channel)(buf & 0x0f);
					nextstate = 1;
				}
				else {		// Invalid channel, ignore.
					nextstate = 0;
				}
			}
			else {		// DATA
				assembleFrame(currentchannel, buf);
				nextstate = 0;
			}
		}
		state = nextstate;
	}
}

void PhysicalLayer::assembleFrame(Channel channel, uint8_t data) {
	static Message * incomingframe[NUM_CHANNELS];
	static uint8_t index[NUM_CHANNELS];
	static uint8_t currentstate[] = { 0, 0, 0, 0 };
	uint8_t nextstate;

	switch (currentstate[channel]){
	case 0:		// Awaiting start indicator.
		if (data != 0xC0) {
			nextstate = 0;
		}
		else {
			// When we enter MEDIUM_IDLE we expect incomingframe to be free, we will not delete old information.
			incomingframe[channel] = new Message(255);	// Allocate a new Message.
			index[channel] = 0;							// reset the buffer overflow prevention counter.
			nextstate = 1;	// Go to data.
		}
		break;
	case 1:		// Receiving data.
		if (index[channel] > 255)
		{
			// Overflow.
			delete incomingframe[channel];
			incomingframe[channel] = NULL;
			nextstate = 3;	// Go to error.
		} 
		else if (data == 0xC0) {
			// End of frame.
			incomingframe[channel]->length = index[channel];
			framequeue[channel].push(incomingframe[channel]);
			incomingframe[channel] = NULL;
			nextstate = 0;	// Go to idle.
		}
		else if (data == 0xDB){
			// Special.
			nextstate = 2;	// Go to special character.
		}
		else {
			// Data.
			incomingframe[channel]->data[index[channel]] = data;
			index[channel]++;
			nextstate = 1;	// Go to data.
		}

		// If count > length message then delete and quit.
		break;
	case 2:		// Special character.
		if (data == 0xDC) {
			incomingframe[channel]->data[index[channel]] = 0xC0;
			index[channel]++;
			nextstate = 1;	// Go to data.
		}
		else if (data == 0xDD){
			incomingframe[channel]->data[index[channel]] = 0xDB;
			index[channel]++;
			nextstate = 1;	// Go to data.
		}
		else {
			delete incomingframe[channel];
			incomingframe[channel] = NULL;
			nextstate = 3;	// Go to error.
		}
		break;
	case 3:		// Error.
		if (data == 0xC0) {
			nextstate = 0;
		}
		else {
			nextstate = 3;
		}
		break;
	}
	currentstate[channel] = nextstate;
}

MediumState PhysicalLayer::getMediumState(Channel channel) {

	if (channel >= NUM_CHANNELS)	// Validate input.
		return MEDIUM_UNKNOWN;

	return this->mediumstate[channel];
}