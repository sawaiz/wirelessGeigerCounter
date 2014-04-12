#define F_CPU 16000000UL
#include <util/delay_basic.h>

uint16_t delay_multiplier = 400 * 5; //Base cycles for 100us is 400
uint16_t delay_ms = 100 * 8;

int pulseCount = 0;
long previousMillis = 0;
int interval = 10000; //Every 10 seconds
byte deviceAddress = B01010001;
byte parity;

int transmitPin = 11;

void countPulse(){
	detachInterrupt(0);
	pulseCount++;

	while (digitalRead(2) == 0){
	}
	attachInterrupt(0, countPulse, FALLING);
}

void setup(){
	Serial.begin(115200);

	pinMode(transmitPin, OUTPUT);
	pinMode(transmitPin - 1, OUTPUT);
	pinMode(transmitPin - 2, OUTPUT);
	digitalWrite(transmitPin - 2, LOW);
	digitalWrite(transmitPin - 1, HIGH);

	attachInterrupt(0, countPulse, FALLING);
}

void loop(){

	unsigned long currentMillis = millis();
	if (currentMillis - previousMillis > interval) {
		detachInterrupt(0);
		previousMillis = currentMillis;

		parity = (deviceAddress^pulseCount) ^ pulseCount >> 8;

		rf_send(deviceAddress);
		rf_send(pulseCount >> 8);
		rf_send(pulseCount);
		rf_send(parity);

		Serial.println(pulseCount);

		pulseCount = 0;
		attachInterrupt(0, countPulse, FALLING);
	}

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
	_delay_loop_2(delay_multiplier * 6);
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


