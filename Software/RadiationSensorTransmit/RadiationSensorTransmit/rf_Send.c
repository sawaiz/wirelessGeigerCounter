#include <avr/io.h>
#include <util/delay_basic.h>
#define DELAY_LENGTH 28*5

#include "rf_Send.h"

int TRANSMIT;

void rf_send_setup(int pin){
	TRANSMIT = pin;
	DDRB |=1<<TRANSMIT;
}

void rf_send(uint8_t byte){
	//Some Pretty bad manchester encoding
	int i;
	for (i = 0; i < 20; i++){
		PORTB |= (1 << TRANSMIT);
		_delay_loop_1(DELAY_LENGTH);
		PORTB &= ~(1 << TRANSMIT); //Sets pin to low (original and (not new))
		_delay_loop_1(DELAY_LENGTH);
	}
	PORTB |= (1 << TRANSMIT);
	_delay_loop_2(288*3);
	PORTB &= ~(1 << TRANSMIT);
	_delay_loop_1(DELAY_LENGTH);

	for ( i = 0; i < 8; i++)
	{
		if (byte & (1 << i)){
			PORTB |= (1 << TRANSMIT);
		}
		else
		{
			PORTB &= ~(1 << TRANSMIT);
		}
		_delay_loop_1(DELAY_LENGTH);

		if (byte & (1 << i)){
			PORTB &= ~(1 << TRANSMIT);
		}
		else
		{
			PORTB |= (1 << TRANSMIT);
		}
		_delay_loop_1(DELAY_LENGTH);
	}
	PORTB &= ~(1 << TRANSMIT);
}