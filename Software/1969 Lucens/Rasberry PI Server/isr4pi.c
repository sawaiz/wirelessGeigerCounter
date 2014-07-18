/*
isr4pi.c
D. Thiebaut
based on isr.c from the WiringPi library, authored by Gordon Henderson
https://github.com/WiringPi/WiringPi/blob/master/examples/isr.c

Compile as follows:

    gcc -o isr4pi isr4pi.c -lwiringPi $(mysql_config --libs)

Run as follows:

    sudo ./isr4pi

 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <mysql/mysql.h>

// Use GPIO Pin 17, which is Pin 0 for wiringPi library

#define ASK_PIN0 0
#define ASK_PIN1 0
#define DELAY_US 500

#define DATABASE_NAME		"gieger"
#define DATABASE_USERNAME	"root"
#define DATABASE_PASSWORD	"Penetent2401"
MYSQL *mysql1;

int good;
int i;
int bufferLoc = 0;
int buffer[4];
int printBuffer = 0;
int cpm = 0;
unsigned char data_in = 'A';

int BitShiftCombine(unsigned char x_high, unsigned char x_low)
{
  int combined;
  combined = x_high;              //send x_high to rightmost 8 bits
  combined = combined << 8;         //shift x_high over to leftmost 8 bits
  combined |= x_low;                 //logical OR keeps x_high intact in combined and fills in rightmost 8 bits
  return combined;
}

void mysql_connect (void)
{
    //initialize MYSQL object for connections
	mysql1 = mysql_init(NULL);

    if(mysql1 == NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(mysql1));
        return;
    }

    //Connect to the database
    if(mysql_real_connect(mysql1, "localhost", DATABASE_USERNAME, DATABASE_PASSWORD, DATABASE_NAME, 0, NULL, 0) == NULL)
    {
    	fprintf(stderr, "%s\n", mysql_error(mysql1));
    }
    else
    {
        printf("Database connection successful.\n");
    }
}

void mysql_disconnect (void)
{
    mysql_close(mysql1);
    printf( "Disconnected from database.\n");
}

void writeCpm(int adress, int cpm){
  if(mysql1 != NULL){
    char query[127];
    sprintf(query, "INSERT INTO cpm (transmitter,counts) VALUES (%d,%d)", adress, cpm);
    if (mysql_query(mysql1, query)){
      fprintf(stderr, "%s\n", mysql_error(mysql1));
      return;
    }
  }
}

// -------------------------------------------------------------------------
// myInterrupt:  called every time an event occurs
void data_incoming(void) {
  for (i = 0; i<100; i++){
    delayMicroseconds(20);
    good = 1;
    if (digitalRead(ASK_PIN1) == LOW){
      good = 0;
      i = 100;
    }
  }//for
  if(good == 1){
    while(1){
      if(digitalRead(ASK_PIN1)== LOW){
        delayMicroseconds(DELAY_US*1.5);
        for(i=0; i<8; i++){
          if(digitalRead(ASK_PIN1)==HIGH){
            data_in |= 1<<i;
          }
          if(digitalRead(ASK_PIN1)==LOW){
            data_in &= ~(1<<i);
          }
          delayMicroseconds(DELAY_US*2);
        }

        if(bufferLoc != 0){
          buffer[bufferLoc] = data_in;
          if(bufferLoc == 3){
            bufferLoc = 0;
            printBuffer = 1;
          }
          else{
            bufferLoc++;
          }
        }
        if(data_in == 0x51){
          buffer[0] = data_in;
          bufferLoc++;
        }
        break;
      }
    }
  }
}


// -------------------------------------------------------------------------
// main
int main(void) {
  // sets up the wiringPi library
  if (wiringPiSetup () < 0) {
      fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno));
      return 1;
  }

  pinMode(ASK_PIN1, INPUT);

  // set Pin 17/0 generate an interrupt on high-to-low transitions
  // and attach myInterrupt() to the interrupt
  if ( wiringPiISR (ASK_PIN0, INT_EDGE_RISING, &data_incoming) < 0 ) {
      fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
      return 1;
  }

  mysql_connect();
//  mysql_disconnect();

  while ( 1 ) {
    if(printBuffer==1){
      printBuffer =0;
      if(((buffer[0]^buffer[1])^buffer[2]) == buffer[3]){
        cpm = BitShiftCombine(buffer[1], buffer[2])*6;
        writeCpm(buffer[0], cpm);
        printf("%d,%d,%d,%d    CPM = %d\n", buffer[0], buffer[1], buffer [2], buffer [3], cpm);
      }
    }
  }

  return 0;
}
