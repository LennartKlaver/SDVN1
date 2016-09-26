/*
 * receiver.c
 *
 * Created: 25-11-2014 11:34:22
 *  Author: Lennart
 */ 
/************************************************************************/
/* LIBRARIES                                                            */
/************************************************************************/
#include "receiver.h"

/************************************************************************/
/* DEFINITIONS                                                          */
/************************************************************************/
// SYSTEMTIME gets updated every 16us seconds.
// SHAPE OF HIGH SYMBOL.
// The minimal width of the high signal.
#define TIMELIMIT_MINIMAL_HIGH_TIME 26
// The maximal width of the high signal.
#define TIMELIMIT_MAXIMAL_HIGH_TIME 44
// SYMBOL DECODING.
// If the limit below is exceeded on receiving an edge we determine that we have a zero.
#define TIMELIMIT_END_OF_ONE    37
// END OF TRANSMISSION DETECTION.
// If the limit below is exceeded without receiving an edge we determine that we do not have a signal.
#define TIMELIMIT_END_OF_SIGNAL 94

// These two depend on the deviation of the noise on top of the signal.
#define THRESHOLDLOW_OFFSET  20
#define THRESHOLDHIGH_OFFSET 10
#define THRESHOLDHIGH_OFFSET_INIT 10

typedef enum {NODATA, LOW, HIGH} SIGNALSTATE_t;
typedef enum {CHANNEL1, CHANNEL2, CHANNEL3, CHANNEL4, NUM_CHANNELS} channel_t;
	
/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/
void process_sample(uint16_t sample, channel_t channel);

/************************************************************************/
/* VARIABLES                                                            */
/************************************************************************/
volatile uint16_t SYSTEMTIME = 0;

uint8_t receiverenabled = 0;

SIGNALSTATE_t rxsignalstate[] = {NODATA, NODATA, NODATA, NODATA};
uint16_t thresholdlow[]     = {0,0,0,0};
uint16_t thresholdhigh[]    = {0,0,0,0};
uint16_t sampleslow[NUM_CHANNELS][8];
uint16_t sampleshigh[NUM_CHANNELS][8];
uint8_t  sampleindexhigh[]  = {0,0,0,0};
uint8_t	 sampleindexlow[]   = {0,0,0,0};
uint16_t timestamp[]        = {0,0,0,0};
uint8_t receivedbyte[]      = {0,0,0,0};
int8_t receivedbitcounter[] = {7,7,7,7};
uint8_t mediumbusy[]		= {0,0,0,0};

// Filtering.
uint16_t zminusone[]		= {0,0,0,0};
uint16_t zminustwo[]		= {0,0,0,0};

/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/
void receiver_init(void) {
	timer0_init();
}

/*
 * Calculate the low threshold.
 */
static inline void updatethresholdlow(uint8_t channel, uint16_t sample) {
	sampleslow[channel][sampleindexlow[channel]] = sample;		// Add the sample to the circular buffer.
	sampleindexlow[channel] = (sampleindexlow[channel] + 1) % 8;	// Update the buffer index in a circular fashion.
	uint16_t mean = calculateMean(sampleslow[channel]);
	thresholdlow[channel] = mean + (mean >> 3); //+ THRESHOLDLOW_OFFSET;	// Set the new threshold.
}

/*
 * Calculate the high threshold.
 */
static inline void updatethresholdhigh(uint8_t channel, uint16_t sample) {
	sampleshigh[channel][sampleindexhigh[channel]] = sample;		// Add the sample to the circular buffer.
	sampleindexhigh[channel] = (sampleindexhigh[channel] + 1) % 8;	// Update the buffer index in a circular fashion.
	uint16_t mean = calculateMean(sampleshigh[channel]);
	thresholdhigh[channel] = ((mean + thresholdlow[channel])>>1) - THRESHOLDHIGH_OFFSET;	// Set the new threshold.
}

/*
 * Filter the data.
 * This filter uses a Median filter to filter sudden spikes and drops. This smoothes the square wave.
 * Without the filter, sometimes the signal is decoded wrong or the decoder is triggered falsely.
 */
inline uint16_t filter(uint8_t channel, uint16_t sample) {
	
	uint16_t result = 0;
	
	// We use a median filter to smooth unexpected pulses.
	// This is actually a sorting filter.
	// For example a spike:
	// [1000 3000 1000] => sorting => [1000 1000 3000] => take the middle value => 1000
	if(zminustwo[channel] > zminusone[channel]) {
		if(zminustwo[channel] > sample) {
			if(zminusone[channel] > sample) {
				result = zminusone[channel];
			} else {
				result = sample;
			}
		} else {
			result = zminustwo[channel];
		}
	} else {
		if(zminustwo[channel]>sample) {
			result = zminustwo[channel];
		} else {
			if(zminusone[channel]>sample){
				result = sample;
			}else {
				result = zminusone[channel];;
			}
		}
	}
	
	// Update the delay variables.
	zminustwo[channel] = zminusone[channel];
	zminusone[channel] = sample;
	
	return result;
}

/**
 * Do a channel measurement.
 */
void receiver_measure() {
	
	// Repeat the measurement.
	for(uint8_t i=0; i<10; i++) {
		uint16_t sample1 = spi_adc_read(ADC_CHANNEL2);
		uint16_t sample2 = spi_adc_read(ADC_CHANNEL3);
		uint16_t sample3 = spi_adc_read(ADC_CHANNEL4);
		uint16_t sample4 = spi_adc_read(ADC_CHANNEL1);
		
		// Calculate the low threshold.
		updatethresholdlow(CHANNEL1, sample1);
		updatethresholdlow(CHANNEL2, sample2);
		updatethresholdlow(CHANNEL3, sample3);
		updatethresholdlow(CHANNEL4, sample4);

		// TODO: determine the noise deviation and use it to set the threshold offsets.
		// TODO: make sure the medium is idle.
	}
	
	// Set thresholdhigh to an initial value.
	thresholdhigh[CHANNEL1] = (thresholdlow[CHANNEL1]<<1) + THRESHOLDHIGH_OFFSET_INIT;
	thresholdhigh[CHANNEL2] = (thresholdlow[CHANNEL2]<<1) + THRESHOLDHIGH_OFFSET_INIT;
	thresholdhigh[CHANNEL3] = (thresholdlow[CHANNEL3]<<1) + THRESHOLDHIGH_OFFSET_INIT;
	thresholdhigh[CHANNEL4] = (thresholdlow[CHANNEL4]<<1) + THRESHOLDHIGH_OFFSET_INIT;
}

void receiver_sample(void) {
	if(receiverenabled){
		process_sample(spi_adc_read(ADC_CHANNEL2), CHANNEL1);
		process_sample(spi_adc_read(ADC_CHANNEL3), CHANNEL2);
		process_sample(spi_adc_read(ADC_CHANNEL4), CHANNEL3);
		process_sample(spi_adc_read(ADC_CHANNEL1), CHANNEL4);
	}
}

/**
 * Handle the ADC samples and decode them.
 */
void process_sample(uint16_t sample, channel_t channel) {	
	SIGNALSTATE_t new_rxsignalstate = NODATA;
	
	// Run a smoothing filter over the data.	
	sample = filter(channel, sample);	
		
	switch (rxsignalstate[channel]) {
		case NODATA:
if(channel == CHANNEL3){
PORTC |= (1<<PORTC2);	// Switch on.
PORTC &= ~(1<<PORTC2);	// Switch off.
}
		// Check if we have a rising edge.
		if (sample > thresholdhigh[channel]) {
			// We have a rising edge.
			receivedbyte[channel] = 0;		// Reset the data variable.
			receivedbitcounter[channel] = 7;	// Reset the bit counter.
			timestamp[channel] = SYSTEMTIME;
			mediumbusy[channel] = 1;	// Set the medium state.
			new_rxsignalstate = HIGH;	// Set the next state.
		}
		else {
			// We are still idle.
			// Do a medium measurement.
			// Calculate the low threshold.
			updatethresholdlow(channel, sample);
			thresholdhigh[channel] = (thresholdlow[channel]<<1); //+ THRESHOLDHIGH_OFFSET_INIT;
			mediumbusy[channel] = 0;	// Set the medium state.
			new_rxsignalstate = NODATA;	// Set the next state.
		}

		break;
		case HIGH:
if(channel == CHANNEL3)
PORTC |= (1<<PORTC2);	// Switch on.
		// Check if we have a falling edge.
		if (sample < thresholdlow[channel]) {
			uint16_t timedif = getTimeDifference(timestamp[channel], SYSTEMTIME);
			if ((timedif > TIMELIMIT_MINIMAL_HIGH_TIME) && (timedif < TIMELIMIT_MAXIMAL_HIGH_TIME)) {
				// We have a falling edge.
				timestamp[channel] = SYSTEMTIME;		// Register the time.
				new_rxsignalstate = LOW;	// Set the next state.
			}
			else {
				// The high symbol is too long, so it is invalid.
				new_rxsignalstate = NODATA;
			}
		}
		else if (sample > thresholdhigh[channel]) {
			// We are still high.
			// Calculate the high threshold.
			updatethresholdhigh(channel, sample);
			new_rxsignalstate = HIGH;	// Set the next state.
		}
		else {
			// We are in between two signal states (a sample on the edge), discard the sample.
			// TODO: Do a measurement if this state happens multiple times.
			new_rxsignalstate = HIGH;	// Stay in the current state.
		}
		break;
		case LOW:
if(channel == CHANNEL3)
PORTC &= ~(1<<PORTC2);	// Switch off.
		// Check if we have a rising edge.
		if (sample > thresholdlow[channel]) {
			// Yes, we have an edge.
			mediumbusy[channel] = 1;	// Set the medium state.
			// Does the time in the interval represent a one or a zero?
			uint16_t result = getTimeDifference(timestamp[channel], SYSTEMTIME);
			if (result > TIMELIMIT_END_OF_ONE) {
				receivedbyte[channel] |= (0 << receivedbitcounter[channel]);	// We have received a zero, write it.
			}
			else {
				receivedbyte[channel] |= (1 << receivedbitcounter[channel]);	// We have received a one, write it.
			}

			// Do we have a complete byte?
			if (!receivedbitcounter[channel]) {
				// Yes, complete, send it over UART.
				uart_write(channel | (mediumbusy[CHANNEL4] << 7) | (mediumbusy[CHANNEL3] << 6) | (mediumbusy[CHANNEL2] << 5) | (mediumbusy[CHANNEL1] << 4));				// Send the receiver address.
				uart_write(receivedbyte[channel]);	// Send the data.
				
				// Reset the bitcounter.
				receivedbyte[channel] = 0;		// Reset the data variable.
				receivedbitcounter[channel] = 7;	// Reset the bit counter.
			}
			else {
				receivedbitcounter[channel]--;		// No, Shift to the next bit.
			}
			timestamp[channel] = SYSTEMTIME;		// Log the time.
			new_rxsignalstate = HIGH;	// Set the next state.
		}
		else if (sample < thresholdlow[channel]) {
			// We do not have a signal edge, so we are still in low signal state.

			// Check if the time has elapsed.
			if (getTimeDifference(timestamp[channel],SYSTEMTIME) > TIMELIMIT_END_OF_SIGNAL) {
				// The signal stays idle, so we have no data.
				// Incomplete data we received is discarded because we should always receive complete 8 bit bytes.
				mediumbusy[channel] = 0;	// Set the medium state.
				new_rxsignalstate = NODATA;	// Set the next state.
			}
			else {
				// No, not yet a time out.
				// Calculate the low threshold.
				updatethresholdlow(channel, sample);
				new_rxsignalstate = LOW;	// Set the next state.
			}
		}
		else {
			// We are in between two signal states (a sample on the edge), discard the sample.
			// TODO: Do a measurement if this state happens multiple times.
			new_rxsignalstate = LOW; // Stay in the current state.
		}
		break;
	}

	rxsignalstate[channel] = new_rxsignalstate;
}

void receiver_reset(void) {
	rxsignalstate[CHANNEL1] = NODATA;
	rxsignalstate[CHANNEL2] = NODATA;
	rxsignalstate[CHANNEL3] = NODATA;
	rxsignalstate[CHANNEL4] = NODATA;
}

void receiver_setenabled(uint8_t enabled) {
	receiverenabled = enabled;
}


/**
 * ISR for overflow interrupt of Timer 0 (8-bit timer).
 */
ISR(TIMER0_OVF_vect) {
	SYSTEMTIME++;	// Update the system counter.
	
	// Enable the lines below to measure the time between two ticks on port C2.
	//PORTC |= (1<<PORTC2);	// Switch on.
	//PORTC &= ~(1<<PORTC2);	// Switch off.
}