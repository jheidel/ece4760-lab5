#define F_CPU 16000000UL  // 16 MHz

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include <util/delay.h>

#define UART_BAUD  1000000 // 1 MBaud (max speed of FDTI chip)

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

#define SUCCESS	0

void startup_flash(void) {
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

void indicate_serial_error(void) {
	for(;;) {
		FLIP(PORTD, PIN_STATUS_LED);
		_delay_ms(100);
	}
}

inline void SPI_TX(uint16_t data)
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
	SPI_TX(0xD002);
}

void uart_setup(void) {

// Setup baud rate & 8n1
#if F_CPU < 2000000UL && defined(U2X)
  UCSR0A = _BV(U2X);             /* improve baud rate error by using 2x clk */
  UBRR0L = (F_CPU / (8UL * UART_BAUD)) - 1;
#else
  UBRR0L = (F_CPU / (16UL * UART_BAUD)) - 1;
#endif
  UCSR0B = _BV(TXEN0) | _BV(RXEN0); /* tx/rx enable */



}

inline char uart_get(uint16_t* ret) {
    *ret = 0;

	loop_until_bit_is_set(UCSR0A, RXC0);
	if (UCSR0A & _BV(FE0))
	  return _FDEV_EOF;
	if (UCSR0A & _BV(DOR0))
	  return _FDEV_ERR;

	*ret |= UDR0 << 8;

	loop_until_bit_is_set(UCSR0A, RXC0);
	if (UCSR0A & _BV(FE0))
	  return _FDEV_EOF;
	if (UCSR0A & _BV(DOR0))
	  return _FDEV_ERR;

	*ret |= UDR0;
	return 0;
}

uint16_t x;
uint16_t y;
char ret;
char blank;

inline void readwrite() {

	ret = uart_get(&x); // TODO check for success
	if (ret != SUCCESS) indicate_serial_error();

	SPI_TX((x & 0x0FFF) | 0x5000); // Set B
	SPI_TX((x & 0x0FFF) | 0x5000); // Do it again (unknown reason; chip bug?)

	ret = uart_get(&y); // TODO check for success
	if (ret != SUCCESS) indicate_serial_error();

	SPI_TX((y & 0x0FFF) | 0xC000); // Set A (update both)
	
	if ((x >> 12) & 0x01) { //blanking bit
		PORTD &= ~((1<<PIN_STATUS_LED) | (1<<PIN_LASER_MOD));
	} else {
		PORTD |= (1<<PIN_STATUS_LED) | (1<<PIN_LASER_MOD);
	}
	
	SPI_TX((y & 0x0FFF) | 0xC000); // Do it again (unknown reason; chip bug?)
}

void initialize(void) {

	DDRD = (1<<PIN_STATUS_LED) | (1<<PIN_LASER_MOD);
	PORTD = 0;

	dac_setup();
	uart_setup();

	startup_flash();
}

int main(void) {

	initialize();

	for(;;) {
		readwrite();
	}
}
