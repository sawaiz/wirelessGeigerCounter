#define F_CPU 16000000UL
#include <util/delay_basic.h>

int i, good, k;
byte data_in;

uint16_t delay_multiplier = 400 * 5; //Base cycles for 100us is 400
uint16_t delay_ms = 100 * 8;

int buffer[4];
int bufferLoc = 0;

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
				
				if (bufferLoc != 0)
				{
					buffer[bufferLoc] = data_in;
					if (bufferLoc == 3)
					{
						bufferLoc = 0;
						for (int i = 0; i < 4; i++)
						{
							Serial.print(buffer[i]);
							Serial.print(",");
						}
						if ((buffer[0]^buffer[1])^buffer[2] == buffer[3])
						{
							Serial.print("     Verified  ");
							int cpm = (BitShiftCombine(buffer[1], buffer[2]) * 6);
							Serial.print(cpm);
							Serial.print(" Counts Per Min");
						}
						Serial.println();
					}
					else{
						bufferLoc++;
					}
				}
				else{


					if (data_in == 0x2F){
						buffer[0] = data_in;
						bufferLoc++;
					}
				}

				break;//secondtwhile
			}//low kickoff

		}//second while

	}//good check

	attachInterrupt(0, data_incoming, RISING);



}//routine

int BitShiftCombine(unsigned char x_high, unsigned char x_low)
{
	int combined;
	combined = x_high;              //send x_high to rightmost 8 bits
	combined = combined << 8;         //shift x_high over to leftmost 8 bits
	combined |= x_low;                 //logical OR keeps x_high intact in combined and fills in                                                             //rightmost 8 bits
	return combined;
}