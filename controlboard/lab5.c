#define F_CPU 16000000UL  // 16 MHz

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include <util/delay.h>

#include "uart.h"

// UART file descriptor
// putchar and getchar are in uart.c
FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

#define PIN_STATUS_LED	PIN2
#define PIN_LASER_MOD	PIN3

// Helper macros
// http://people.ece.cornell.edu/land/courses/ece4760/Debugging/index.htm
#define READ(U, N) ((U) >> (N) & 1u)
#define SET(U, N) ((void)((U) |= 1u << (N)))
#define CLR(U, N) ((void)((U) &= ~(1u << (N))))
#define FLIP(U, N) ((void)((U) ^= 1u << (N))) 

#define DDR_SPI	DDRB
#define DD_MOSI	3
#define DD_SCK	5
#define PIN_CS	2

void debug_flash(void) {
		SET(PORTD, 2);
		_delay_ms(20);
		CLR(PORTD, 2);
		_delay_ms(50);
		SET(PORTD, 2);
		_delay_ms(20);
		CLR(PORTD, 2);
		_delay_ms(50);
		SET(PORTD, 2);
		_delay_ms(20);
		CLR(PORTD, 2);
}


void SPI_TX(uint16_t data)
{
	/* Acquire CS */
	PORTB &= ~(1<<PIN_CS);

	/* Start transmission */
	SPDR = (data >> 8) & 0xFF;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));

	SPDR = data & 0xFF;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));

	/* Release CS */
	PORTB |= (1<<PIN_CS);
}

void dac_setup(void) {
	/* INITIALIZE SPI */
	
	/* Set MOSI and SCK output, all others input */
	DDR_SPI = (1<<DD_MOSI) | (1<<DD_SCK) | (1<<PIN_CS);
	/* Enable SPI, Master, set clock rate fck/2 w/ CPHA=1 */
	SPCR = (1<<SPE) | (1<<MSTR) | (1<<CPHA);
	SPSR = (1<<SPI2X);
	
	PORTB = (1<<PIN_CS);
	_delay_ms(100); // Setup delay before sending initial SPI command

	// Transmit control
	// Set 2.048V internal reference on ref pin
	// Set slow mode
	SPI_TX(0x9002);
}




void dac_write(uint16_t aValue, uint16_t bValue) {
	/* TODO: writing to buffer isn't working... */
	SPI_TX((bValue & 0x0FFF) | 0x1000); // Set B
	SPI_TX((aValue & 0x0FFF) | 0x8000); // Set A (update both)
}


void initialize(void) {

	DDRD = (1<<PIN_STATUS_LED) | (1<<PIN_LASER_MOD);
	PORTD = 0;

	dac_setup();

	uart_init();
	stdout = stdin = stderr = &uart_str;

	fprintf(stdout, "\n\n== LASER CONTROLLER ==\n");

}

uint16_t i;
int main(void) {

	initialize();

	for (;;) {
		fprintf(stdout, "Enter DAC value (between 0 and 4095):\n> ");
		fscanf(stdin, "%d", &i);

		if (i < 0x0000 || i > 0x0FFF) {
			fprintf(stdout, "VALUE %d OUT OF RANGE\n", i);
		} else {
			fprintf(stdout, "Setting DAC channels to %d\n", i);
			dac_write(i, i);
			debug_flash();
		}

	}
}
