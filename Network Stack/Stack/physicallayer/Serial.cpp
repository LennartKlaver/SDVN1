#include "Serial.h"

Serial::Serial(char * portName){
	//We're not yet connected
	this->connected = false;

	//Try to connect to the given port throuh CreateFile
	this->serialHandle = CreateFileA(portName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	//Check if the connection was successfull
	if (this->serialHandle == INVALID_HANDLE_VALUE)
	{
		//If not success full display an Error
		if (GetLastError() == ERROR_FILE_NOT_FOUND){

			//Print Error if neccessary
			printf("ERROR: Handle was not attached. Reason: %s not available.\n", portName);

		}
		else
		{
			printf("ERROR!!!");
		}
	}
	else
	{
		//If connected we try to set the comm parameters
		DCB dcbSerialParams = { 0 };

		//Try to get the current
		if (!GetCommState(this->serialHandle, &dcbSerialParams))
		{
			//If impossible, show an error
			printf("failed to get current serial parameters!");
		}
		else
		{
			//Define serial connection parameters for the board
			dcbSerialParams.BaudRate = CBR_115200;
			dcbSerialParams.ByteSize = 8;
			dcbSerialParams.StopBits = ONESTOPBIT;
			dcbSerialParams.Parity = NOPARITY;

			//Set the parameters and check for their proper application
			if (!SetCommState(serialHandle, &dcbSerialParams))
			{
				printf("ALERT: Could not set Serial Port parameters");
			}
			else
			{
				//If everything went fine we're connected
				this->connected = true;
				//We wait 2s as the board will be reseting
				Sleep(SERIAL_WAIT_TIME);
			}
		}
	}
}

Serial::~Serial() {
	//Check if we are connected before trying to disconnect
	if (this->connected)
	{
		//We're no longer connected
		this->connected = false;
		//Close the serial handler
		CloseHandle(this->serialHandle);
	}
}

uint8_t Serial::IsConnected(void) {
	//Simply return the connection status
	return this->connected;
}

uint16_t Serial::read(uint8_t * buf, uint16_t length) {
	//Number of bytes we'll have read
	DWORD bytesRead;

	//Use the ClearCommError function to get status info on the Serial port
	ClearCommError(this->serialHandle, &this->errors, &this->status);

	//Check if there is something to read
	if (this->status.cbInQue >= length)
	{
		//Try to read the require number of chars, and return the number of read bytes on success
		ReadFile(this->serialHandle, buf, length, &bytesRead, NULL);
		
		return bytesRead;

	}

	//If nothing has been read, or that an error was detected return -1
	return 0;
}

uint8_t Serial::write(uint8_t * buf, uint16_t length) {

	DWORD bytesSend;

	//Try to write the buffer on the Serial port
	if (!WriteFile(this->serialHandle, (void *)buf, length, &bytesSend, 0))
	{
		//In case it don't work get comm error and return false
		ClearCommError(this->serialHandle, &this->errors, &this->status);

		return false;
	}
	else
		return true;
}

