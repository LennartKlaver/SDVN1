/*
 * transmitter.h
 *
 * Created: 23-11-2014 15:53:52
 *  Author: Lennart
 */ 
#ifndef TRANSMITTER_H_
#define TRANSMITTER_H_

/************************************************************************/
/* LIBRARIES                                                            */
/************************************************************************/
#include <stdint.h>
#include "../spi/spi.h"
#include "../uart/RingBuffer.h"
#include "../receiver/receiver.h"

/************************************************************************/
/* DEFINITIONS                                                          */
/************************************************************************/

/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/
void transmitter_init(void);
void transmitter_timertick(void);
void transmitter_add(uint8_t data);
void transmitter_setdirection(uint8_t led8_1, uint8_t led16_9, uint8_t led20_17);

/************************************************************************/
/* INLINE FUNCTIONS                                                     */
/************************************************************************/


#endif /* TRANSMITTER_H_ */