#include <avr/io.h>
#include <avr/interrupt.h> 
#include <avr/sleep.h>
//#include <avr/power.h>
#include "rf_Send.h"

#define POT_PIN PB4
#define BUTTON_PIN PB1
#define TRANSMIT PB2

ISR(INT0_vect)
{
	cli();
//	int i;
//	uint8_t message[] = "Caledonia#";	
//	for(i = 0; i<sizeof(message);i ++ ){
//		rf_send(message[i]);
//	}
	uint8_t potVal = ADCH;
	rf_send(potVal);
	sei();
}

int main(void) {
	PORTB = 1 << BUTTON_PIN;		// Enable Button pull-up resistor
	
	ADCSRA |= 1<<ADPS2;				//Sets prescalar to 16, giving a 75khz sampling
	ADMUX |= 1<<REFS0;				//Internal Voltage Reference
	ADMUX |= 1<<MUX1;				//ADC2 (PB4) Selection on the Input Channel
	ADMUX |= 1<<ADLAR;				//Sets accuracy to 8 bit
	
	ADCSRA |= 1<<ADEN;				//Enable ADC
	ADCSRA |= 1<<ADSC;				//Start Conversion
	
	rf_send_setup(TRANSMIT);
	
	GIMSK = 1 << INT0; 				// Enable INT0 (PB1 pin 6)
	MCUCR = 0 << ISC00;			// Trigger INT0 on low level
	MCUCR |= 0 << ISC01;
	
	sei();							//Enable Global Interrupt
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	
	while (1) {
		sleep_mode();  				 // go to sleep and wait for interrupt...
	}
	return 0;
}