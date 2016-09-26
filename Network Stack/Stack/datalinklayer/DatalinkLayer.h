/**
* \class DatalinkLayer
*
* \brief This class does Medium Access Control and data interpretation/extraction from raw frames.
*
* This class does Medium Access Control and data interpretation/extraction from raw frames.
* It runs inside its own thread so it can respond to basic service requests like ping.
* Its most important task is to create the one-hop communication. To accomplish this, the Datalink Layer has neighbor awareness and access control. 
* To sense the surrounding, the Datalink Layer can do a * neighbor discovery and administers the angle to this neighbor. 
* Please note that neighbors are nodes that are within transmission reach, so only one-hop communication.
*
* \note
*
* \author $Author: L.P. Klaver $
* \version $Revision: 1.0 $
* \date $Date: 2014/12/12 16:07:00 $
*
*/
#pragma once

#include <thread>

#include "../physicallayer/PhysicalLayer.h"
#include "../datatypes/Angle.h"
#include "../datatypes/WriteResult.h"
#include "../datatypes/Message.h"
#include "../datatypes/ReceiverMessage.h"
#include "../datatypes/Address.h"
#include "../datatypes/Neighbor.h"
#include "../datatypes/MediumState.h"
#include "../datatypes/MessageType.h"
#include "../datatypes/NetworkMessage.h"
#include "../physicallayer/Queue.h"

#include <vector>

using std::vector;

class DatalinkLayer
{
private:
	std::thread readThread;		//< Object that contains our read thread.
	bool _continue;				//< This variable can make the thread stop or start remotely.
	Queue<NetworkMessage *> messagequeue;	//< This queue stores received messages after processing by the Datalink Layer.
	Queue <Neighbor *> pingreplyqueue;	//< This queue stores received ping replies for further handling by the Datalink Layer.
	Angle startangle[4];		//< "Global" to define the angles of the receivers.
	Angle endangle[4];			//< "Global" to define the angles of the receivers.
	static const uint16_t	MAC_BACKOFF_RETRYLIMIT = 3; //< Number of retries before a timeout.
	PhysicalLayer *			physical;	//< A pointer to the physical layer object so we can send our raw data somewhere.
	Address					myaddress;	//< My MAC address.
	vector<Neighbor>		neighbors;	//< A list of my neighbors.

	struct findNeighbor : std::unary_function<Neighbor, bool> {
		Address				address;
		findNeighbor(Address a) : address(a) { }
		bool operator()(Neighbor const& n) const {
			return (n.address == address);
		}
	};
	/** Method to calculate how long we should wait before doing a new transmission attempt.
	  * \param attemptcount The number of attempts we have tried already.
	  * \return The number of milliseconds to wait. 
	  */
	uint16_t	calculateBackoffDelay(uint16_t attemptcount);
	Message *	createPingRequestMessage(Address destination);
	Message *	createPingAnswerMessage(Address destination);
	Message *	createNetworkInformationMessage(Address destination);
	void		processIncomingMessage(Message * msg, Channel ch);
	uint8_t		getLengthWithParity(uint8_t length);

public:
	DatalinkLayer(PhysicalLayer * phy, Address address);
	~DatalinkLayer();

	/** Write data to a certain node.
	* \param destination The address of the designated neighbor.
	* \param message The data to transmit.
	* \return Success if succeeded or failed if failed.
	*/
	WriteResult			write(Address destination, Message * message);
	WriteResult			send(Address destination, Message * message);
	WriteResult			send(Angle startangle, Angle endangle, Message * message);
	NetworkMessage *	read(void);

	/** Method to discover neighbors around the board using a given angle step increase.
	* \param step The angle steps each discovery should increase.
	* \return The number of discovered neighbors.
	*/
	Queue<Address> *	discoverNeighbors(Angle step);

	/** Method to discover neighbors in a certain direction.
	* \param startangle The start angle of the direction to transmit in.
	* \param endangle The end angle of the direction to transmit in.
	* \return true if something is found or false if not.
	*/
	Queue<Address> *	discoverNeighbors(Angle startangle, Angle endangle);

	/** Method to register a neighbor on a certain direction.
	* If the neighbor is new, we add it and return true. If it already exists then we try to update its position and return false.
	* \param address The address of the neighbor.
	* \param startangle The start angle of the direction.
	* \param endangle The end angle of the direction.
	* \return true if it is a new neighbor or false if it already existed.
	*/
	bool				registerNeighbor(Address address, Angle startangle, Angle endangle);

	/** Method that needs to be run in a thread. This makes the processing asynchroneous.
	* (In Windows it is very hard to get an interrupt if data is available, hence this system).
	*/
	void				processIncomingData(void);
};

