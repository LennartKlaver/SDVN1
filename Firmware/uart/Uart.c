/*
* Serial.c
*
* Created: 5-6-2014 19:53:06
*  Author: Lennart
*/
#include "uart.h"
#include "RingBuffer.h"

#include <stdint.h>
#include <avr/interrupt.h>

//#define MYUBRR       ((F_CPU/16/BAUD_RATE)-1)
#define MYUBRR       ((F_CPU/8/BAUD_RATE)-1) // When speed doubling is used select this <<

// buffers for use with the ring buffer (belong to the UART)
RingBuff_t ring_buffer_out;
RingBuff_t ring_buffer_in;

uint8_t uart_read() {
	return RingBuffer_Remove(&ring_buffer_in);
}

void uart_write(uint8_t data) {
	// Disable interrupts to get exclusive access to ring_buffer_out.
	cli();
	if (RingBuffer_IsEmpty(&ring_buffer_out)) {
		// First data in buffer, enable data ready interrupt
		UCSR0B |=  (1 << UDRIE0);
	}
	
	if(!RingBuffer_IsFull(&ring_buffer_out)) {
		// Put data in buffer
		RingBuffer_Insert(&ring_buffer_out, data);
	}
	
	// Re-enable interrupts
	sei();
}

uint8_t uart_writebuffer_ready () {
	return !RingBuffer_IsFull(&ring_buffer_out);
}

uint8_t uart_readbuffer_ready () {
	return !RingBuffer_IsFull(&ring_buffer_in);
}

uint8_t uart_char_waiting() {
	return !RingBuffer_IsEmpty(&ring_buffer_in);
}

void uart_init() {

	// Set the baud rate.
	UBRR0H = (uint8_t) (MYUBRR>>8);
	UBRR0L = (uint8_t) MYUBRR;
	
	// Enable this bit for UART speed doubling.
	// UCSR0A |= (1 << U2X0);
	UCSR0A = 0b00000010;
	
	// enable RX and TX and set interrupts on rx complete
	// UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	UCSR0B = 0b10011000;
	
	// 8-bit, 1 stop bit, no parity, asynchronous UART
	// UCSR0C = (1 << UCSZ01) | (1 << UCSZ00) | (0 << USBS0) |
	// (0 << UPM01) | (0 << UPM00) | (0 << UMSEL01) |
	// (0 << UMSEL00);
	UCSR0C = 0b00000110;
	
	// initialize the in and out buffer for the UART
	RingBuffer_InitBuffer(&ring_buffer_in);
	RingBuffer_InitBuffer(&ring_buffer_out);
	
}

uint8_t uart_getreceivedcount() {
	return RingBuffer_GetCount(&ring_buffer_in);
}


void uart_clearreceivedbuffer() {
	RingBuffer_InitBuffer(&ring_buffer_in);
}

ISR(USART_RX_vect) {
	RingBuffer_Insert(&ring_buffer_in, UDR0);
}

ISR(USART_UDRE_vect) {
	// if there is data in the ring buffer, fetch it and send it
	if (!RingBuffer_IsEmpty(&ring_buffer_out)) {
		UDR0 = RingBuffer_Remove(&ring_buffer_out);
	}
	else {
		// no more data to send, turn off data ready interrupt
		UCSR0B &= ~(1 << UDRIE0);
	}
}
