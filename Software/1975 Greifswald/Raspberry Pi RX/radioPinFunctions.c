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

#include <bcm2835.h>

#define set_bit(PIN) bcm2835_gpio_write(PIN, HIGH)
#define clr_bit(PIN) bcm2835_gpio_write(PIN, LOW)
#define check_bit(PIN) bcm2835_gpio_lev(PIN)

#define CE RPI_V2_GPIO_P1_08
#define CSN RPI_V2_GPIO_P1_07
#define SCK RPI_V2_GPIO_P1_11
#define MOSI RPI_V2_GPIO_P1_10
#define MISO RPI_V2_GPIO_P1_09 

/* ------------------------------------------------------------------------- */
void nrf24_setupPins()
{
    if (!bcm2835_init())
        return 1;

    bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_OUTP); // CE output
    bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_OUTP); // CSN output
    bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_OUTP); // SCK output
    bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_OUTP); // MOSI output
    bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_INPT); // MISO input
}
/* ------------------------------------------------------------------------- */
void nrf24_ce_digitalWrite(uint8_t state)
{
    if(state)
    {
        set_bit(CE);
    }
    else
    {
        clr_bit(CE);
    }
}
/* ------------------------------------------------------------------------- */
void nrf24_csn_digitalWrite(uint8_t state)
{
    if(state)
    {
        set_bit(CSN);
    }
    else
    {
        clr_bit(CSN);
    }
}
/* ------------------------------------------------------------------------- */
void nrf24_sck_digitalWrite(uint8_t state)
{
    if(state)
    {
        set_bit(SCK);
    }
    else
    {
        clr_bit(SCK);
    }
}
/* ------------------------------------------------------------------------- */
void nrf24_mosi_digitalWrite(uint8_t state)
{
    if(state)
    {
        set_bit(MOSI);
    }
    else
    {
        clr_bit(MOSI);
    }
}
/* ------------------------------------------------------------------------- */
uint8_t nrf24_miso_digitalRead()
{
    return check_bit(MISO);
}
/* ------------------------------------------------------------------------- */
