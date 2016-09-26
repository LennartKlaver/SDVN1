/*
 * atmega328p_spi.c
 *
 * Created: 3-4-2014 14:12:31
 *  Author: Lennart
 */ 
/************************************************************************/
/* LIBRARIES                                                            */
/************************************************************************/
#include "spi.h"

/************************************************************************/
/* DEFINITIONS                                                          */
/************************************************************************/

/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/

/**
 * Function SPI_ReadWrite
 * This function sends dataout to the selected slave and returns data that it has received at the same time.
 */
__inline uint8_t spi_readwrite(uint8_t dataout) {
	
    SPDR = dataout;	// Start transmission to the slave.

    while(!(SPSR & (1<<SPIF)));	// Wait for the transmission to complete.
	
    return SPDR;	// Return the incoming data.
}

/** 
 * Function SPI_GPIO_Write
 * Contains the procedure to send a key-value pair to the GPIO expander.
 */
void spi_gpio_write(uint8_t address, uint8_t data) {

	SPI_PORT &= ~(1<<SPI_CS_GPIO);	// Enable the GPIO module.
	
	// Send the 2 bytes to the GPIO module.	
	SPDR = address;					// Start transmission to the slave.
	while(!(SPSR & (1<<SPIF)));		// Wait for the transmission to complete.
	
	SPDR = data;					// Start transmission to the slave.
	while(!(SPSR & (1<<SPIF)));		// Wait for the transmission to complete.
	
	
	SPI_PORT |= (1<<SPI_CS_GPIO);	// Disable the GPIO module.
}

/**
 * Read the sample from the adc, and select the next sample channel for the next read.
 */
uint16_t spi_adc_read(uint8_t channel) {
				
	SPI_PORT &= ~(1<<SPI_CS_ADC);	// Enable the ADC module.
	
	// Send the 2 bytes to the ADC module.
	uint8_t high = spi_readwrite(channel);
	uint8_t low  = spi_readwrite(0);
	
	SPI_PORT |= (1<<SPI_CS_ADC);	// Disable the ADC module.
	
	return ((high<<8) | (low));
}

/**
 * Run a quick test to see if the SPI GPIO comm is working.
 */
void spi_gpio_test() {
	
	spi_gpio_write(GPIO_BANK_LED8_1, 0x00);		// All off.
	spi_gpio_write(GPIO_BANK_LED16_9, 0x00);	// All off.
	spi_gpio_write(GPIO_BANK_LED20_17, 0x00);	// All off.

	spi_gpio_write(GPIO_BANK_LED8_1, 0xff);
	spi_gpio_write(GPIO_BANK_LED16_9, 0xff);
	spi_gpio_write(GPIO_BANK_LED20_17, 0xff);
	
	_delay_ms(2500);
	
	spi_gpio_write(GPIO_BANK_LED8_1, 0x00);		// All off.
	spi_gpio_write(GPIO_BANK_LED16_9, 0x00);	// All off.
	spi_gpio_write(GPIO_BANK_LED20_17, 0x00);	// All off.
}