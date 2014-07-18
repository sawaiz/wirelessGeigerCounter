/*
* ----------------------------------------------------------------------------
* “THE COFFEEWARE LICENSE” (Revision 1):
* <ihsan@kehribar.me> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you think
* this stuff is worth it, you can buy me a coffee in return.
* -----------------------------------------------------------------------------
* Please define your platform spesific functions in this file ...
* -----------------------------------------------------------------------------
*/

#include <avr/io.h>

#define set_bit(reg,bit) reg |= (1<<bit)
#define clr_bit(reg,bit) reg &= ~(1<<bit)
#define check_bit(reg,bit) (reg&(1<<bit))

/* ------------------------------------------------------------------------- */
void nrf24_setupPins()
{
    set_bit(DDRD,6); // CE output
    set_bit(DDRD,5); // CSN output
    set_bit(DDRB,0); // SCK output
    set_bit(DDRD,4); // MOSI output
    clr_bit(DDRB,1); // MISO input
}
/* ------------------------------------------------------------------------- */
void nrf24_ce_digitalWrite(uint8_t state)
{
    if(state)
    {
        set_bit(PORTD,6);
    }
    else
    {
        clr_bit(PORTD,6);
    }
}
/* ------------------------------------------------------------------------- */
void nrf24_csn_digitalWrite(uint8_t state)
{
    if(state)
    {
        set_bit(PORTD,5);
    }
    else
    {
        clr_bit(PORTD,5);
    }
}
/* ------------------------------------------------------------------------- */
void nrf24_sck_digitalWrite(uint8_t state)
{
    if(state)
    {
        set_bit(PORTB,0);
    }
    else
    {
        clr_bit(PORTB,0);
    }
}
/* ------------------------------------------------------------------------- */
void nrf24_mosi_digitalWrite(uint8_t state)
{
    if(state)
    {
        set_bit(PORTD,4);
    }
    else
    {
        clr_bit(PORTD,4);
    }
}
/* ------------------------------------------------------------------------- */
uint8_t nrf24_miso_digitalRead()
{
    return check_bit(PINB,1);
}
/* ------------------------------------------------------------------------- */
