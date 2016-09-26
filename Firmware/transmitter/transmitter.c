/*
 * transmitter.c
 *
 * Created: 25-11-2014 9:45:09
 *  Author: Lennart
 */ 
/************************************************************************/
/* LIBRARIES                                                            */
/************************************************************************/
#include "transmitter.h"

/************************************************************************/
/* DEFINITIONS                                                          */
/************************************************************************/
typedef enum {TXSTATE_DISABLED, TXSTATE_SIGNALRISE, TXSTATE_SIGNALFALL, TXSTATE_SIGNALLOW} TXSTATE;
	
/************************************************************************/
/* VARIABLES                                                            */
/************************************************************************/
RingBuff_t ring_buffer_data;

// Current selected transmission direction.
uint8_t txLed8_1	= 0b01110000;
uint8_t txLed16_9	= 0x00;
uint8_t txLed20_17	= 0x00;

TXSTATE TX1state = TXSTATE_DISABLED;

uint8_t currentbyte = 0;
int8_t  dataindex	= 7;

/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/

/**
 * Process the transmitter state machine at a predefined interval.
 */
void transmitter_timertick(void) {
	TXSTATE TX1nextstate = TXSTATE_DISABLED;
	
	switch(TX1state) {
		
		case TXSTATE_DISABLED:
			if(RingBuffer_IsEmpty(&ring_buffer_data)) {		// Check if there is new data available.
				TX1nextstate = TXSTATE_DISABLED;			// No, go to disabled state.
				receiver_setenabled(1);
			}
			else {
				currentbyte = RingBuffer_Remove(&ring_buffer_data);	// Yes, load a new byte.
				dataindex = 7;						// Reset the bit finger.
				TX1nextstate = TXSTATE_SIGNALRISE;	// Set the next state.
				
				// Disable the receiver.
				receiver_setenabled(0);
				receiver_reset();
			}
		break;
		
		// Rising edge, __|``
		case TXSTATE_SIGNALRISE:
			spi_gpio_write(GPIO_BANK_LED8_1,	txLed8_1);	// Set the selected leds on.
			spi_gpio_write(GPIO_BANK_LED16_9,	txLed16_9);	// Set the selected leds on.
			spi_gpio_write(GPIO_BANK_LED20_17,	txLed20_17);	// Set the selected leds on.
//DEBUG
//PORTC |= (1<<PORTC2);	// Switch on.
			TX1nextstate = TXSTATE_SIGNALFALL;	// Set the next state.
		break;
		
		// Falling edge, ``|_
		case TXSTATE_SIGNALFALL:
			spi_gpio_write(GPIO_BANK_LED8_1,	0);	// Set the leds off.
			spi_gpio_write(GPIO_BANK_LED16_9,	0);	// Set the leds off.
			spi_gpio_write(GPIO_BANK_LED20_17,	0);	// Set the leds off.
//DEBUG
//PORTC &= ~(1<<PORTC2);	// Switch off.

			// Check if we finished transmitting the current byte.
			if(dataindex < 0) {
				// Current byte is transmitted, load the next byte if possible.
				if(RingBuffer_IsEmpty(&ring_buffer_data)) {		// Check if there is new data available.
					TX1nextstate = TXSTATE_DISABLED;			// No, go to disabled state.
					break;
				}
				else {
					// Data available, load a new byte.
					currentbyte =  RingBuffer_Remove(&ring_buffer_data);
					dataindex = 7;	// Reset the bit finger.
					TX1nextstate = TXSTATE_SIGNALRISE;	// Go to next state.
				}
				
			}
		
			// Check if the data bit is a one or a zero.
			if((currentbyte >> dataindex) & 0x1) {		
				// A one, we are done, proceed to the next bit.
				TX1nextstate = TXSTATE_SIGNALRISE;	// Set the next state.
			}
			else {
				// A zero, generate another low period.
				TX1nextstate = TXSTATE_SIGNALLOW;	// Set the next state.
			}
			dataindex--;	// Move the bit finger to the next bit.
		break;
		
		// Keeping the signal low for one tick.
		case TXSTATE_SIGNALLOW:
			// Signals are already low, so do nothing with them.
			// This state just absorbs one timer tick to signal a low.
			TX1nextstate = TXSTATE_SIGNALRISE;	// Set the next state.
		break;
		
		default:
			// Invalid state reached.
			// TODO: handle invalid state.
		break;
	}
	TX1state = TX1nextstate;	// Go to next state.	
}

/**
 * Initialize the transmitter.
 */
void transmitter_init(void) {

	RingBuffer_InitBuffer(&ring_buffer_data); // initialize the buffer.
}

/* Functions regarding the data buffer. */

/**
 * Add a byte to the transmitter queue.
 */
void transmitter_add(uint8_t data) {
	
	if(!RingBuffer_IsFull(&ring_buffer_data)) {
		RingBuffer_Insert(&ring_buffer_data, data);	// Put data in the buffer.
	} else {
		//TODO: handle a full buffer. Currently we just discard the data.
	}
}

/**
 * Set the direction to transmit to.
 */
void transmitter_setdirection(uint8_t led8_1, uint8_t led16_9, uint8_t led20_17) {
	txLed8_1	= led8_1;
	txLed16_9	= led16_9;
	txLed20_17	= led20_17;
}