//main.c
//Sawaiz Syed
//Recive data from NRF Module
//And possibly Do some good with it
//
//compile with
// gcc -o main main.c nrf24.c radioPinFunctions.c -l bcm2835
// sudo ./main
//
//Maybe I even need to include
//NRF in that statement

#include <bcm2835.h>
#include <stdio.h>
#include <stdint.h>
#include "nrf24.h"

int main(int argc, char **argv){

    printf("testing?");

    uint8_t data_array[4];
    uint8_t tx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};
    uint8_t rx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};

    if (!bcm2835_init())
        return 1;

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
            data_array[3]);
        }
    }
    bcm2835_close();
    return 0;
}
