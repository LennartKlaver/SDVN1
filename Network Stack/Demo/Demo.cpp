#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

#include "physicallayer/PhysicalLayer.h"
#include "datalinklayer/DatalinkLayer.h"
#include "networklayer/NetworkLayer.h"
#include "physicallayer/Serial.h"
#include "datatypes/Message.h"


class Demo
{
public:
	Serial * serialport;
	PhysicalLayer * phy;
	DatalinkLayer * dll;
	NetworkLayer * nl;

	Demo(char * comport, Address address) {
		// Init the UART communication.
		char str[12];
		strcpy_s(str, "\\\\.\\");
		strcat_s(str, comport);

		serialport = new Serial(str);
		phy = new PhysicalLayer(serialport);
		dll = new DatalinkLayer(phy, address);
		nl  = new NetworkLayer(dll, address);
	}

	~Demo() {
		delete nl;
		delete dll;
		delete phy;
		delete serialport;
	}
};


int main()
{
	Demo * demo1 = new Demo("COM4", 1);
	Demo * demo2 = new Demo("COM3", 2);
	Demo * demo3 = new Demo("COM7", 3);

	// INITIALIZE.
	// 1 --> 2 --> 3.
//	demo1->nl->registerNeighbor(2, 250, 290);


//	demo2->nl->registerNeighbor(1, 45, 135);
//	demo2->nl->registerNeighbor(3, 250, 290);

	int foundnodes = demo1->nl->discoverNetwork(18);
	if (foundnodes > 0)
		printf("Node 1 discovered new nodes: %d\n", foundnodes);

	foundnodes = demo2->nl->discoverNetwork(18);
	if (foundnodes > 0)
		printf("Node 2 discovered new nodes: %d\n", foundnodes);
		
	demo1->nl->registerNode(2, 3);

	system("pause");

	// RUN SOMETHING.
	Message * msg = new Message(13);
	strcpy((char *)msg->data, "Hello World!");
	
	demo1->nl->write(3, msg);

	delete msg;

	system("pause");

	NetworkMessage * netmsg = demo3->nl->read();
	if (netmsg) {
		printf("%s\n", netmsg->msg->data);
		delete netmsg;
	}

	system("pause");

	delete demo1;
	delete demo2;
	delete demo3;
}

