#define F_CPU 16000000UL
#include <util/delay_basic.h>

int i, good, k;
byte data_in;

uint16_t delay_multiplier = 400 * 5; //Base cycles for 100us is 400
uint16_t delay_ms = 100 * 8;

int last_recived;
int errors = 0;

void setup(){
	attachInterrupt(0, data_incoming, RISING);

	pinMode(1, OUTPUT);
	pinMode(4, OUTPUT);

	digitalWrite(1, LOW);
	digitalWrite(4, HIGH);

	pinMode(2, INPUT);
	Serial.begin(115200);
}//setup

void loop(){


}//loop


void data_incoming(){


	for (i = 0; i<100; i++){
		delayMicroseconds(20);
		good = 1;
		if (digitalRead(2) == LOW){
			good = 0;
			i = 100;
		}
	}//for

	if (good == 1){
		detachInterrupt(0);
		while (1){
			if (digitalRead(2) == LOW){
				//delayMicroseconds(delay_ms*1.5);
				_delay_loop_2(delay_multiplier*1.5);

				for (i = 0; i<8; i++){
					if (digitalRead(2) == HIGH)
						bitWrite(data_in, i, 1);
					else
						bitWrite(data_in, i, 0);
					//delayMicroseconds(delay_ms*2);
					_delay_loop_2(delay_multiplier * 2);
				}//for
				Serial.print(data_in);

				if (data_in != last_recived + 1){
					errors++;
				}

				if (data_in == 255){
					last_recived = -1;
				}
				else{
					last_recived = data_in;
				}
				Serial.print("    Errors: ");
				Serial.println(errors);

				break;//secondtwhile
			}//low kickoff

		}//second while

	}//good check

	attachInterrupt(0, data_incoming, RISING);



}//routine