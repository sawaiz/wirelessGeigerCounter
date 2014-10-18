//main.c
//Sawaiz Syed
//Recive data from NRF Module
//And possibly Do some good with it
//
//compile with
// gcc -o blink main.c -l bcm2835
// sudo ./blink
//
//Maybe I even need to include
//NRF in that statement

#include <bcm2835.h>
#include <stdio.h>
#include "nrf24.h"

uint8_t temp;
uint8_t q = 0;
uint8_t data_array[4];
uint8_t tx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};
uint8_t rx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};

int main(){
    /* init hardware pins */
    nrf24_init();

    /* Channel #2 , payload length: 4 */
    nrf24_config(2,4);

    /* Set the device addresses */
    nrf24_tx_address(tx_address);
    nrf24_rx_address(rx_address);

    printf("Configured");

    while(1){
        if(nrf24_dataReady())
        {
            nrf24_getData(data_array);
            printf("> %d %d %d %d \n",
            data_array[0],
            data_array[1],
            data_array[2],
            data_array[3];)
        }
    }
}
