/*
 * atmega328p_spi.h
 *
 * Created: 3-4-2014 14:12:43
 *  Author: Lennart
 */ 
#ifndef _spi_h
#define _spi_h

/************************************************************************/
/* LIBRARIES                                                            */
/************************************************************************/
#include <avr/io.h>
#include <util/delay.h>

/************************************************************************/
/* DEFINITIONS                                                          */
/************************************************************************/
#define SPI_PORT     PORTB
#define SPI_CS_GPIO  PORTB1
#define SPI_CS_ADC   PORTB2
#define SPI_MISO     PORTB4
#define SPI_MOSI     PORTB3
#define SPI_SCK      PORTB5

//ADC
#define ADC_CHANNEL1	0b00000000
#define ADC_CHANNEL2	0b00001000
#define ADC_CHANNEL3	0b00010000
#define ADC_CHANNEL4	0b00011000

//GPIO
#define GPIO_BANK_LED8_1	0x4C
#define GPIO_BANK_LED16_9	0x54
#define GPIO_BANK_LED20_17	0x5C

/************************************************************************/
/* PROTOTYES                                                            */
/************************************************************************/
void		spi_gpio_write(uint8_t address, uint8_t data);
uint16_t	spi_adc_read(uint8_t next_channel);
void		spi_gpio_test(void);

/************************************************************************/
/* INLINE FUNCTIONS                                                     */
/************************************************************************/

/**
 * Initialize the SPI module.
 * pre:
 * post: The SPI module is ready for use.
 */
__inline void spi_init() {
	
	// Set the SPI output pins as outputs in the Data Direction Register B, others are inputs.
	//DDRB |= (1<<SPI_SCK);
	//DDRB |= (1<<SPI_MOSI);
	//DDRB |= (1<<SPI_CS_ADC);
	//DDRB |= (1<<SPI_CS_GPIO);
	DDRB = 0b00101110;

	// Disable the chips.
	SPI_PORT |= (1<<SPI_CS_ADC);
	SPI_PORT |= (1<<SPI_CS_GPIO);

	// Enable the SPI module, set it as Bus Master and set the speed.
	//SPCR  = (1<<SPE);		// Enable SPI.
	//SPCR |= (1<<MSTR);	// Set SPI in Master Mode.
	SPCR = 0b01010000;
	
	//SPSR |= (1<<SPI2X);	// Set SPI in double speed Mode (max speed Fosc/2).
	SPSR = 0b00000001;
}

/**
 * Initialize the ADC through SPI.
 */
__inline void spi_adc_init(){
	
	spi_adc_read(ADC_CHANNEL1);	// Set the ADC to read channel 1.
}

/**
 * Initialize the GPIO expander.
 * pre: The SPI module is initialized.
 * post: The SPI GPIO expander pins are set as outputs and ready to switch.
 * TODO: do something with the result value.
 */
__inline void spi_gpio_init() {
	
	spi_gpio_write(0x04,0x01);	// Set the configuration to Enabled.
	
	// Set virtual pins as output to reduce power.
	spi_gpio_write(0x09,0x55);	// Set pin P4 - P7 as output.
	spi_gpio_write(0x0A,0x55);	// Set pin P8 - P11 as output.
	
	// Set normal pins as output.
	spi_gpio_write(0x0B, 0x55);	// Set pin P12 - P15 as output.	
	spi_gpio_write(0x0C, 0x55);	// Set pin P16 - P19 as output.
	spi_gpio_write(0x0D, 0x55);	// Set pin P20 - P23 as output.
	spi_gpio_write(0x0E, 0x55);	// Set pin P24 - P27 as output.
	spi_gpio_write(0x0F, 0x55);	// Set pin P28 - P31 as output.
}

#endif
