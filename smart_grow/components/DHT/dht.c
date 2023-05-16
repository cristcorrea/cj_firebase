#include <stdio.h>
#include <esp_err.h>
#include <stdint.h>
#include <sys/types.h>
#include <esp_log.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "dht.h"
#include "header.h"

#define MAXdhtData 5	// to complete 40 = 5*8 Bits

#define DHT_OK 0
#define DHT_CHECKSUM_ERROR -1
#define DHT_TIMEOUT_ERROR -2

#define gpio_num 16

static const char* TAG = "DHT_start";

int intentos = 0;

extern sensor_data mediciones; 


int getSignalLevel( int usTimeOut, bool state )
{

	int uSec = 0;
	while( gpio_get_level(gpio_num)==state ) {

		if( uSec > usTimeOut )
			return -1;

		++uSec;
		esp_rom_delay_us(1);//ets_delay_us(1);		// uSec delay
	}

	return uSec;
}

void DHTerrorHandler(int response)
{
	switch(response) {

		case DHT_TIMEOUT_ERROR :
			intentos++;
			ESP_LOGE(TAG,"Sensor Timeout. Repitiendo medición %i.\n", intentos);
			break;

		case DHT_CHECKSUM_ERROR:
			ESP_LOGE(TAG,"Sensor CHECKSUM. Repitiendo medición %i.\n", intentos);
			break;

		case DHT_OK:
			intentos = 0; 
			break;

		default :
			ESP_LOGE(TAG,"Sensor Unknown. Repitiendo medición %i.\n", intentos);
	}
}


int readDHT()
{
	int uSec = 0;

	uint8_t dhtData[MAXdhtData];

	for (int k = 0; k < MAXdhtData; k++)
	{
		dhtData[k] = 0;
	}
	// == Send start signal to DHT sensor ===========

	gpio_set_direction( gpio_num, GPIO_MODE_OUTPUT );

	// 20 ms en cero para activar el dht11 (se necesitan 18 ms sengun datasheet)
	gpio_set_level(gpio_num, 0);
	esp_rom_delay_us( 20000 );

	// pull up por 25 us a la espera de datos (20 a 40 us segun datasheet)
	gpio_set_level( gpio_num, 1 );
	esp_rom_delay_us( 25 );

	gpio_set_direction( gpio_num, GPIO_MODE_INPUT );		

	// DHT pone a low la linea por 80 us y luego en high por 80 us 
	// -- 80us low ------------------------

	uSec = getSignalLevel( 85, 0 );

	if( uSec<0 )
	{
		ESP_LOGE(TAG, "DHT_TIMEOUT_ERROR 1");
		return DHT_TIMEOUT_ERROR;
	} 

	// -- 80us up ------------------------

	uSec = getSignalLevel( 85, 1 );
	if( uSec<0 ) 
	{
		ESP_LOGE(TAG, "DHT_TIMEOUT_ERROR 2");
		return DHT_TIMEOUT_ERROR;
	}
	// == Si no hay errores lee los 40 data bits ================

	for( int k = 0; k < 40; k++ ) 
	{

		// -- starts new data transmission with >50us low signal

		uSec = getSignalLevel( 56, 0 );
		if( uSec<0 )
		{
			ESP_LOGE(TAG, "DHT_TIMEOUT_ERROR 3");
			return DHT_TIMEOUT_ERROR;
		}
		// -- check to see if after >70us rx data is a 0 or a 1

		uSec = getSignalLevel( 75, 1 );
		if( uSec<0 ) 
		{
			ESP_LOGE(TAG, "DHT_TIMEOUT_ERROR 4");
			return DHT_TIMEOUT_ERROR;
		}

		// since all dhtData array where set to 0 at the start,
		// only look for "1" (>28us us)

		if (uSec > 30)
		{
			dhtData[k/8] |= (1 << (7-(k%8))); // 1 << 7, 1 << 6, 1 << 5, ..., 1 << 0
		}

	}


	mediciones.humedad_amb = dhtData[0];			

	mediciones.temperatura_amb = dhtData[2];
	mediciones.temperatura_amb *= 10;
	mediciones.temperatura_amb += dhtData[3];
	mediciones.temperatura_amb /= 10; 

	

	if( dhtData[2] & 0x80 ) 			// negative temp, brrr it's freezing
			mediciones.temperatura_amb *= -1;


	if (dhtData[4] == ((dhtData[0] + dhtData[1] + dhtData[2] + dhtData[3]) & 0xFF))
		return DHT_OK;

	else
		return DHT_CHECKSUM_ERROR;

}