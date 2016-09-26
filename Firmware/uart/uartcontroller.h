/*
 * UartController.h
 *
 * Created: 30-11-2014 16:37:59
 *  Author: Lennart
 */ 
#ifndef UARTCONTROLLER_H_
#define UARTCONTROLLER_H_

/************************************************************************/
/* LIBRARIES                                                            */
/************************************************************************/
#include "../transmitter/transmitter.h"

/************************************************************************/
/* DEFINITIONS                                                          */
/************************************************************************/
typedef enum {UARTSTATE_MENU, UARTSTATE_DATA, UARTSTATE_LED1, UARTSTATE_LED2, UARTSTATE_LED3} UARTSTATE;
#define UART_CONTROL_FRAMELIMITER 0xC0
	
/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

/************************************************************************/
/* INLINE FUNCTIONS                                                     */
/************************************************************************/
inline static void uartcontroller_process(uint8_t byte) {
	static UARTSTATE menustate = UARTSTATE_MENU;
	static uint8_t led8_1 = 0, led16_9 = 0;
	
	UARTSTATE nextstate = UARTSTATE_MENU;
	
	// UART MENU	
	switch(menustate){
		case UARTSTATE_MENU:
				// We have a new transmission.
				if(byte == UART_CONTROL_FRAMELIMITER) {
					transmitter_add(byte);
					nextstate = UARTSTATE_LED1;		//..the next byte will contain the first part of the direction.
				}
				else {
					nextstate = UARTSTATE_MENU;		// No new transmission, ignore.
				}
			break;
		case UARTSTATE_LED1:
				led8_1 = byte;
				nextstate = UARTSTATE_LED2;
			break;
		case UARTSTATE_LED2:
				led16_9 = byte;
				nextstate = UARTSTATE_LED3;
				break;
		case UARTSTATE_LED3:
				transmitter_setdirection(led8_1, led16_9, (byte & 0x0f));		// Set the transmission direction.
				//transmitter_add(byte);				// Signal a new transmission.
				nextstate = UARTSTATE_DATA;
			break;
		case UARTSTATE_DATA:
				transmitter_add(byte);				// Add the byte to the transmitter queue.
				if(byte == UART_CONTROL_FRAMELIMITER) {
					nextstate = UARTSTATE_MENU;		// We have received all the data bytes, quit..
				}
				else {
					nextstate = UARTSTATE_DATA;		// We are receiving a stream, continue.
				}
			break;
		default:
			nextstate = UARTSTATE_MENU;				// Something went wrong, so ignore all incoming bytes and wait for a new transmission frame.		
	}
	menustate = nextstate;
}


#endif /* UARTCONTROLLER_H_ */