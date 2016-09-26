/**
 * \class Serial
 * 
 * \brief This class makes the serial port available.
 *
 * This class provides function to connect to the serial port. It makes the data from the serial port available through read and write functions.
 *
 * \note This is currently implemented for MS Windows, but porting this to Linux should not be hard.
 *
 * \author $Author: L.P. Klaver $
 * \version $Revision: 1.0 $
 * \date $Date: 2014/12/09 15:24:00 $
 * 
 */
#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <stdexcept>

#define SERIAL_WAIT_TIME 2000

class Serial {

private:
	HANDLE serialHandle;	//< the serial comm handler
	bool connected;			//< the current connection status.
	COMSTAT status;			//< the current comm status.
	DWORD errors;			//< the current error status.

public:

	/** Constructor of the Serial class.
	  * \param portname the name of the port to use, e.g. "COM1"
	  */
	Serial(char * portname);

	/** Destructor of the Serial class.
	  * Calling this closes the connection with the OS.
	  */
	~Serial();

	/** This method returns the current connection status.
	  * \return true if still connected or false if not.
	  */
	uint8_t IsConnected(void);

	/** \brief Write data to the serial port.
	  * \param buf the buffer containing data.
	  * \param length the number of bytes in buf.
	  * \return true if the write was successfull or false if not.
	  */
	uint8_t write(uint8_t * buf, uint16_t length);

	/** \brief Read from the serial port.
	  * \param buf the buffer to write to.
	  * \param length the number of bytes desired to read.
	  * \return the number of bytes truly read (less or equal to length).
	  * 
	  * This method reads from the OS serial queue and places that data in the buf if there 
	  * is data available. If less data is in the os queue then this number is placed.
	  * If no data is available the method returns 0.
	  */
	uint16_t read(uint8_t * buf, uint16_t length);

};

#endif // SERIAL_H