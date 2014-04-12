#define F_CPU 16000000UL
#include <util/delay_basic.h>

uint16_t delay_multiplier = 400 * 5; //Base cycles for 100us is 400
uint16_t delay_ms = 100 * 8;

int i;
int transmitPin = 2;

void setup(){
	pinMode(transmitPin, OUTPUT);
	pinMode(transmitPin-1, OUTPUT);
	pinMode(transmitPin-2, OUTPUT);
	digitalWrite(transmitPin-2, LOW);
	digitalWrite(transmitPin-1, HIGH);
}

void loop(){

	i++;
	rf_send(i);
}

void rf_send(byte input){
	int i;

	for (i = 0; i<20; i++){
		digitalWrite(transmitPin, HIGH);
		_delay_loop_2(delay_multiplier);
		digitalWrite(transmitPin, LOW);
		_delay_loop_2(delay_multiplier);
	}

	digitalWrite(transmitPin, HIGH);
	_delay_loop_2(delay_multiplier*6);
	digitalWrite(transmitPin, LOW);
	_delay_loop_2(delay_multiplier);


	for (i = 0; i<8; i++){
		if (bitRead(input, i) == 1)
			digitalWrite(transmitPin, HIGH);
		else
			digitalWrite(transmitPin, LOW);
		//delayMicroseconds(delay_ms);
		_delay_loop_2(delay_multiplier);

		if (bitRead(input, i) == 1)
			digitalWrite(transmitPin, LOW);
		else
			digitalWrite(transmitPin, HIGH);
		//delayMicroseconds(delay_ms);
		_delay_loop_2(delay_multiplier);
	}//i


	digitalWrite(transmitPin, LOW);
}


