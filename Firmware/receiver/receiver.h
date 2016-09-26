/*
 * receiver.h
 *
 * Created: 23-11-2014 17:11:28
 *  Author: Lennart
 */ 
#ifndef RECEIVER_H_
#define RECEIVER_H_

/************************************************************************/
/* LIBRARIES                                                            */
/************************************************************************/
#include <avr/interrupt.h>
#include <stdint.h>
#include "../spi/spi.h"
#include "../timer/timer.h"
#include "../uart/uart.h"

/************************************************************************/
/* DEFINITIONS                                                          */
/************************************************************************/

/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/
void receiver_init(void);
void receiver_sample(void);
void receiver_measure(void);
void receiver_reset(void);
void receiver_setenabled(uint8_t enabled);

/************************************************************************/
/* INLINE FUNCTIONS                                                     */
/************************************************************************/
static inline uint16_t getTimeDifference(uint16_t timestamp, uint16_t currenttime) {

	if (timestamp > currenttime) {
		// Timer has wrapped around.
		return (65535 - (timestamp - currenttime) + 1);
	}
	return (currenttime - timestamp);
}

static inline uint16_t calculateMean(uint16_t * buffer) {
	uint16_t result = 0;

	for (uint8_t i = 0; i<8; i++)
	result += buffer[i];

	return result >> 3;
}

#endif /* RECEIVER_H_ */