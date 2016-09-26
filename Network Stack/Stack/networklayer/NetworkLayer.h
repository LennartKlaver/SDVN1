/**
* \class NetworkLayer
*
* \brief This class provides routing through a network of nodes with multi-hop distances.
*
* This class provides routing through a network of nodes with multi-hop distances.
*
* \note
*
* \author $Author: L.P. Klaver $
* \version $Revision: 1.0 $
* \date $Date: 2014/12/13 12:58:00 $
*
*/
#pragma once

#include "../datatypes/NetworkMessage.h"
#include "../datatypes/WriteResult.h"
#include "../datatypes/Angle.h"
#include "../datatypes/Neighbor.h"
#include "../datalinklayer/DatalinkLayer.h"
#include "../physicallayer/Queue.h"
#include "Router.h"

#include <vector>

using std::vector;

class NetworkLayer
{
private:
	DatalinkLayer *   dll;					//< A pointer to the data link layer object so we can send and receive data.
	Router           router;				//< A router object used for routing through the network.
	vector<Neighbor> neighbors;				//< A software representation of the network.
	Queue<NetworkMessage *> messagequeue;	//< This queue stores received messages after processing by the Network Layer.

	std::thread workThread;	//< This thread makes it possible to do asynchroneous processing. A Windows thingy.
	bool _continue;			//< This variable controls the workThread.

	struct findNeighbor : std::unary_function<Neighbor, bool> {
		Address address;
		findNeighbor(Address a) : address(a) { }
		bool operator()(Neighbor const& n) const {
			return (n.address == address);
		}
	};
	void processIncomingMessage(NetworkMessage * msg);
	WriteResult NetworkLayer::write(Address destination, Address source, Message * msg);
	Address myaddress;

public:
	NetworkLayer(DatalinkLayer * datalinklayer, Address ownaddress);
	~NetworkLayer();

	NetworkMessage * read(void);
	WriteResult		 write(Address destination, Message * msg);
	bool			 registerNeighbor(Address address, Angle startangle, Angle endangle);
	bool			 registerNode(Address parent, Address neighbor);
	uint16_t		 discoverNetwork(Angle step);
	uint16_t		 discoverNetwork(Angle startangle, Angle endangle);

	/** Method that needs to be run in a thread. This makes the processing asynchroneous.
	* (In Windows it is very hard to get an interrupt if data is available, hence this system).
	*/
	void				processIncomingData(void);
};

