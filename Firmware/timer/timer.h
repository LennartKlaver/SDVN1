/*
 * timer.h
 *
 * Created: 21-11-2014 11:07:23
 *  Author: Lennart
 */ 

#ifndef TIMER_H_
#define TIMER_H_

__inline void timer1_init() {
	
	// Collected these commands in one assignment.
	// TCCR1B |= (1<<WGM12);	// Configure timer 1 for CTC mode.
	//
	// Clock scaler selection.
	//	TCCR1B |= (1<<CS10);				// Start timer at Fcpu/1.
	//	TCCR1B |= (1<<CS11);				// Start timer at Fcpu/8.
	//	TCCR1B |= ((1<<CS11) | (1<< CS10));	// Start timer at Fcpu/64.
	//	TCCR1B |= (1<<CS12);				// Start timer at Fcpu/256.
	//	TCCR1B |= ((1<<CS12) | (1<< CS10));	// Start timer at Fcpu/1024.
	TCCR1B = 0b00001001;
	
	TIMSK1 = 0b00000010;	// TIMSK1 |= (1<<OCIE1A);	// Enable CTC interrupt.
		
	// Counter value selection.
	// OCR1A = Fclk / (clock_divider * required_interrupt_frequency).
	OCR1A =	8000;	// Set CTC compare value to 1000 Hz at 16Mhz.
}

__inline void timer0_init() {
		
	// Collected these commands in one assignment.
	// Clock scaler selection.
	//	TCCR0B |= (1<<CS00);				// Start timer at Fcpu/1.
	//	TCCR0B |= (1<<CS01);				// Start timer at Fcpu/8.
	//	TCCR0B |= ((1<<CS00) | (1<< CS01));	// Start timer at Fcpu/64.
	//	TCCR0B |= (1<<CS02);				// Start timer at Fcpu/256.
	//	TCCR0B |= ((1<<CS02) | (1<< CS00));	// Start timer at Fcpu/1024.
	TCCR0B = 0b00000001;
		
	TIMSK0 = 0b00000001;	// TIMSK1 |= (1<<OCIE1A);	// Enable Overflow interrupt.
}


#endif /* TIMER_H_ */