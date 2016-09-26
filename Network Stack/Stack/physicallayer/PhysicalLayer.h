/**
* \class PhysicalLayer
*
* \brief This class forms the interface between data frames and raw bytes.
*
* This class forms the interface between data frames and raw bytes, making it possible to transmit and receive data. 
* To make processing easier the Physical Layer handles data frames and passes them between processing applications and a serial connection.
*
* The PhysicalLayer uses the Serial Line Internet Protocol to communicate with the serial port.
* http://www.tcpipguide.com/free/t_SerialLineInternetProtocolSLIP-2.htm
* This means that the output stream is formatted using delimiters. Because these delimiters can also occur in the data, a coding scheme is used.
* This scheme replaces occurances in the data by special characters.
*
* \note No data interpretation happens in this layer. It just creates streams from data frames and vice versa.
*
* \author $Author: L.P. Klaver $
* \version $Revision: 1.0 $
* \date $Date: 2014/12/09 15:24:00 $
*
*/
#pragma once

#include <stdint.h>
#include "../datatypes/WriteResult.h"
#include "../datatypes/Message.h"
#include "../datatypes/Angle.h"
#include "../datatypes/Channel.h"
#include "../datatypes/ReceiverMessage.h"
#include "../datatypes/MediumState.h"
#include "Serial.h"
#include "Queue.h"

class PhysicalLayer
{
private:
	Serial * serialport;	//< The serial port to which the data streams are written.
	std::thread readThread;	//< This thread makes it possible to do asynchroneous processing. A Windows thingy.
	bool _continue;			//< This variable controls the readThread.
	Queue<Message *> framequeue[NUM_CHANNELS];	//< Frames recovered from the data stream go in this lockprotected queue.
	MediumState mediumstate[NUM_CHANNELS]; //< The current frame state.

	/** Method to get three bytes that represent the angle in LEDs.
	  * \param startangle The start angle.
	  * \param endangle The end angle.
	  * \return The selected LEDs in the variables LED8_1, LED16_9 and led20_17
	  */
	void convertAngleToLEDBytes(Angle startangle, Angle endangle, uint8_t * LED8_1, uint8_t * LED16_9, uint8_t * led20_17);

	/** This method collects bytes until a valid frame is assembled. It then puts that frame into the queue.
	  * \param channel the channel on which the data was received.
	  * \param data the received unprocessed byte.
	  */
	void assembleFrame(Channel channel, uint8_t data);

public:
	/** Constructor of the PhysicalLayer class.
	  * \param conn Pointer to a Serial object.
	  */
	PhysicalLayer(Serial * conn);

	/** Destructor of the PhysicalLayer class.
	  */
	~PhysicalLayer();

	/** Method for writing a frame to the serial output.
	  * \param msg the frame to transmit.
	  * \param startangle The start angle.
	  * \param endangle The end angle.
	  * \return The result of the transmission (1 if successfull, 0 if not).
	  */
	WriteResult write(Message * msg, Angle startangle, Angle endangle);

	/** Method for reading a frame from the serial output if a frame is available.
	  * \param channel The channel to read from.
	  * \return A pointer to the frame or null if not available. The reading object is responsible for destruction.
	  */
	Message * read(Channel channel);

	/** Method to see if we are receiving data in a certain direction.
	  * \param channel The channel to read from.
	  * \return The medium state of that channel.
	  */
	MediumState getMediumState(Channel channel);

	/** Method that needs to be run in a thread. This makes the read process asynchroneous.
	  * (In Windows it is very hard to get an interrupt if data is available, hence this system).
	  */
	void processSerialData(void);
	
};

