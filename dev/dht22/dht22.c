#include "contiki.h"
#include "reg.h"
#include "dev/ioc.h"
#include "dev/sys-ctrl.h"
#include "dev/ssi.h"
#include "dev/gpio.h"

void dht22_init(void) {

}

// return temperature in Celcius
float dht22_get_temperature(void) {
  float temp;
  return temp;
}

// return humidity in %RH
float dht22_get_humidity(void) {
  float hum;
  return hum;
}
