#ifndef DHT22_H_
#define DHT22_H_

#include "contiki.h"

#include <stdint.h>

void dht22_init(void);

// return temperature in Celcius
float dht22_get_temperature(void);

// return humidity in %RH
float dht22_get_humidity(void);

#endif
