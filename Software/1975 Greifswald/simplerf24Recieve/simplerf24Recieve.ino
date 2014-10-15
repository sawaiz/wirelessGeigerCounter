#include "nrf24.h"

uint8_t temp;
uint8_t q = 0;
uint8_t data_array[4];
uint8_t tx_address[5] = { 0xE7, 0xE7, 0xE7, 0xE7, 0xE7 };
uint8_t rx_address[5] = { 0xD7, 0xD7, 0xD7, 0xD7, 0xD7 };

void setup()
{
	Serial.begin(9600);
	/* init hardware pins */
	nrf24_init();

	/* Channel #2 , payload length: 4 */
	nrf24_config(2, 4);

	/* Set the device addresses */
	nrf24_tx_address(tx_address);
	nrf24_rx_address(rx_address);

	Serial.println("Configured");

}

void loop()
{
	if (nrf24_dataReady())
	{
		nrf24_getData(data_array);
		Serial.print("> ");
		Serial.print(data_array[0]);
		Serial.print(" ");
		Serial.print(data_array[1]);
		Serial.print(" ");
		Serial.print(data_array[2]);
		Serial.print(" ");
		Serial.println(data_array[3]);
	}
}
