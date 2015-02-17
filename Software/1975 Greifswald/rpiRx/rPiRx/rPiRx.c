//rPiRx.c
//Sawaiz Syed
//Take data from NRF Module, and display
//Send data to SQL database
//
//
//Requires libmariadbclient bcm2835
//compile with
// gcc -o main main.c nrf24.c radioPinFunctions.c -l bcm2835
// sudo ./main


/*
CREATE DATABASE gieger;
USE gieger;
CREATE TABLE cpm ( id INT(3), genre INT(3), highByte INT(3), lowByte INT(3), time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP);
INSERT INTO cpm (id,genre,highByte,lowByte) VALUES (11,230,34,0);
*/

/*
pacman -Syu
pacman-db-upgrade
pacman -S mysql

cd '/usr' ; /usr/bin/mysqld_safe --datadir='/var/lib/mysql'
/usr/bin/mysql_secure_installation

iptables -L -n
iptables -A INPUT -i eth0 -p tcp --destination-port 3306 -j ACCEPT

mysql -u root -p
MariaDB [(none)]> GRANT ALL PRIVILEGES ON *.* TO 'sawaiz'@'%';
MariaDB [mysql]> USE mysql;
MariaDB [mysql]> SET PASSWORD FOR 'sawaiz'@'%' =PASSWORD('Guilty343');
*/


//Maybe I even need to include
//NRF in that statement

#include <bcm2835.h>
#include <stdio.h>
#include <stdint.h>
#include <mysql/mysql.h>
#include "nrf24l01/nrf24.h"

#define DATABASE_NAME		"gieger"
#define DATABASE_USERNAME	"sawaiz"
#define DATABASE_PASSWORD	"Guilty343"
#define DATABASE_URL	"45.56.114.100"
MYSQL *mysql1;

void mysql_connect(void)
{
	//initialize MYSQL object for connections
	mysql1 = mysql_init(NULL);

	if (mysql1 == NULL)
	{
		fprintf(stderr, "%s\n", mysql_error(mysql1));
		return;
	}

	//Connect to the database
	if (mysql_real_connect(mysql1, DATABASE_URL, DATABASE_USERNAME, DATABASE_PASSWORD, DATABASE_NAME, 0, NULL, 0) == NULL)
	{
		fprintf(stderr, "%s\n", mysql_error(mysql1));
	}
	else
	{
		printf("Database connection successful.\n");
	}
}

void mysql_disconnect(void)
{
	mysql_close(mysql1);
	printf("Disconnected from database.\n");
}

void writeCpm(int id, int genre, int highByte, int lowByte){
	if (mysql1 != NULL){
		char query[127];
		sprintf(query, "INSERT INTO cpm (id,genre,highByte,lowByte) VALUES (%d,%d,%d,%d)", id , genre , highByte , lowByte);
		if (mysql_query(mysql1, query)){
			fprintf(stderr, "%s\n", mysql_error(mysql1));
			return;
		}
	}
}

int main(int argc, char **argv){

	uint8_t data_array[4];
	uint8_t tx_address[5] = { 0xE7, 0xE7, 0xE7, 0xE7, 0xE7 };
	uint8_t rx_address[5] = { 0xD7, 0xD7, 0xD7, 0xD7, 0xD7 };

	if (!bcm2835_init())
		return 1;

	mysql_connect();

	/* init hardware pins */
	nrf24_init();

	/* Channel #2 , payload length: 4 */
	nrf24_config(2, 4);

	/* Set the device addresses */
	nrf24_tx_address(tx_address);
	nrf24_rx_address(rx_address);

	printf("Configured\n");

	while (1){
		if (nrf24_dataReady())
		{
			nrf24_getData(data_array);
			printf("> %d %d %d %d \n",
				data_array[0],
				data_array[1],
				data_array[2],
				data_array[3]);
			writeCpm(
				data_array[0],
				data_array[1],
				data_array[2],
				data_array[3]);
		}
	}
	mysql_disconnect();
	bcm2835_close();
	return 0;
}
