/*
 * _1975_Greifswald.c
 *
 * Created: 07/17/14 23:03:39
 *  Author: Sawaiz
 */ 

#define F_CPU 1000000UL
#define deviceID 0xF0

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "nrf24.h"

uint16_t events = 0;
uint8_t data_array[4];
uint8_t tx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};
uint8_t rx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};

ISR(INT0_vect)
{
	cli();
	PORTB ^= 1<<PORTB3;
	while (!(PIND & 1<<PIND2)){
	}
	events++;
	sei();
}

void pwm_init(){
	// initialize TCCR0 as per requirement, say as follows
	TCCR0A |= (1<<WGM00)|(1<<COM0A1)|(1<<WGM01);
	TCCR0B |= (1<<CS00);
	TCCR0B |= (0<<CS01);
	TCCR0B |= (0<<CS02);
	
	// make sure to make OC0 as output pin
	DDRB |= (1<<PORTB2);
	
	OCR0A = 125; //Set to 400V
}

void interupt_init(){
	GIMSK = 1 << INT0; 			// Enable INT0
	MCUCR |= 0 << ISC00;		// Trigger INT0 on low level
	MCUCR |= 0 << ISC01;
	DDRD |= 0 << PORTD2;		//Make Sure its an input as well.
	sei();						//enable global interrupts
}

int main(){
	DDRB |= 1<<PORTB3;
	PORTB = 1<<PORTB3;
	
	/* init hardware pins */
	nrf24_init();
	
	/* Channel #2 , payload length: 4 */
	nrf24_config(2,4);

	/* Set the device addresses */
	nrf24_tx_address(tx_address);
	nrf24_rx_address(rx_address);
	
	pwm_init();				// initialize timer in PWM mode
	interupt_init();		//initalize Interrupt 0
	
	/* Fill the data buffer */
	data_array[0] = deviceID;
	data_array[1] = 0xAA;
	
	while(1){
		data_array[3] = events;
		data_array[2] = events >> 8;
		nrf24_send(data_array);
		
// 		/* Wait for transmission to end */
// 		while(nrf24_isSending());
		
		_delay_ms(1000);
	}
}