//avrTx.c
//Sawaiz, Aaron

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "nrf24/nrf24.h"

#define deviceID 0x33
#define ledPin PIND3
#define ledPort DDRD

uint16_t events = 0;
uint8_t data_array[4];
uint8_t tx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};
uint8_t rx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};

ISR(INT0_vect) {
 cli();
 PORTB ^= 1<<PORTB3;
 while (!(PIND & 1<<PIND2)){
 }
 events++;
 sei();
}

void pwm_init() {
 TCCR0A |= (1<<WGM00);
 TCCR0A |= (1<<WGM01);
 TCCR0A |= (1<<COM0A1);
 TCCR0B |= (1<<CS00);
 // make sure to make OC0 as output pin
 DDRB |= (1<<PORTB2);
 OCR0A = 127;          //Set to 400V
}

void interupt_init() {
 GIMSK = 1 << INT0;    // Enable INT0
 MCUCR |= 0 << ISC00;  // Trigger INT0 on low level
 MCUCR |= 0 << ISC01;
 DDRD |= 0 << PORTD2;  //Make Sure its an input as well.
 sei();                //enable global interrupts
}

int main() {
 DDRB |= 1<<PORTB3;
 PORTB |= 1<<PORTB3;

 ledPort |= _BV(ledPin);

 /* init hardware pins */
 nrf24_init();

 /* Channel #2 , payload length: 4 */
 nrf24_config(2,4);

 /* Set the device addresses */
 nrf24_tx_address(tx_address);
 nrf24_rx_address(rx_address);

 pwm_init();   // initialize timer in PWM mode
 interupt_init();  //initalize Interrupt 0

 /* Fill the data buffer */
 data_array[0] = deviceID;
 data_array[1] = 0xAA;

 while(1) {
  data_array[3] = events;
  data_array[2] = events >> 8;
  nrf24_send(data_array);

  /* Wait for transmission to end */
  while(nrf24_isSending()) {
  }

  ledPort &= ~_BV(ledPin);
  events = 0;
  _delay_ms(1000);
  ledPort |= _BV(ledPin);
 }
}
