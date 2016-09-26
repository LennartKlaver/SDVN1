/*
 * atmega328p_uart.h
 *
 * Created: 3-4-2014 14:09:03
 *  Author: Lennart
 */ 
#pragma once

#include <stdint.h>
/************************************************************************/
/* DEFINITIONS                                                          */
/************************************************************************/

/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/
void	uart_write(uint8_t byte);
uint8_t uart_read(void);
uint8_t uart_char_waiting(void);
uint8_t uart_writebuffer_ready(void);
uint8_t uart_readbuffer_ready(void);
void	uart_init(void);
uint8_t uart_getreceivedcount(void);
void    uart_clearreceivedbuffer();