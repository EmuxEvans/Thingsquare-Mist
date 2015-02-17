#include <string.h>
#include "mist.h"
#include "dev/button-sensor.h"
#include "dev/board.h"
#include "dev/i2c.h"
#include "netstack-aes.h"
#include "../../dev/sht21/sht21.h"
#include "websocket.h"
// #include "../../dev/ds18b20/ds18b20.h"
// #include "../../dev/ds18b20/owhal.h"

char ctemp[20];
char chum[20];
int isReady = 0;

static struct websocket s;
static char sensing_payload[350];
static uip_ipaddr_t google_ipv4_dns_server = {
    .u8 = {
      /* Google's IPv4 DNS in mapped in an IPv6 address (::ffff:8.8.8.8) */
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0xff, 0xff,
      0x08, 0x08, 0x08, 0x08,
    }
};

// #define HOST "198.41.30.241" // m2m.eclipse.org
#define HOST "64.94.18.120"
#define API_KEY "B2S0Un5o2VTChjpVWyzElnxxehXjr8N9KSA9YG02HC5arqmd"
#define FEED_ID "582702676"
#define SERIAL "V3CXZ99ZX7K6"

static void callback(struct websocket *s, websocket_result r,
                     uint8_t *data, uint16_t datalen);

#define RECONNECT_INTERVAL 60 * CLOCK_SECOND
static struct ctimer reconnect_timer;

/*---------------------------------------------------------------------------*/
PROCESS(websocket_example_process, "Websocket Example");
PROCESS(environode_process, "Environode sensing and sending");
AUTOSTART_PROCESSES(&environode_process);
/*---------------------------------------------------------------------------*/
static void
reconnect_callback(void *ptr)
{
  websocket_open(&s, "ws://64.94.18.120:8080/",
                 NULL, callback);
}
/*---------------------------------------------------------------------------*/
static void
callback(struct websocket *s, websocket_result r,
         uint8_t *data, uint16_t datalen)
{
  if(r == WEBSOCKET_CLOSED ||
     r == WEBSOCKET_RESET ||
     r == WEBSOCKET_HOSTNAME_NOT_FOUND ||
     r == WEBSOCKET_TIMEDOUT) {
    ctimer_set(&reconnect_timer, RECONNECT_INTERVAL, reconnect_callback, s);
  } else if(r == WEBSOCKET_CONNECTED) {
    websocket_send_str(s, "Connected");
  } else if(r == WEBSOCKET_DATA) {
    printf("websocket-example: Received data '%.*s' (len %d)\n", datalen,
           data, datalen);
  }
}

/*---------------------------------------------------------------------------*/
static void
route_callback(int event, uip_ipaddr_t *route,
               uip_ipaddr_t *ipaddr, int numroutes)
{
  if (event == UIP_DS6_NOTIFICATION_DEFRT_ADD) {
    //leds_off(LEDS_ALL);
    printf("APP - Got an RPL route\r\n");
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(websocket_example_process, ev, data)
{
  static struct etimer et;
  PROCESS_BEGIN();

  ctimer_set(&reconnect_timer, RECONNECT_INTERVAL, reconnect_callback, &s);

  while(1) {
    etimer_set(&et, CLOCK_SECOND * 30);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    sprintf(sensing_payload, "%s%s%s%s%s%s%s%s%s", "{\"method\":\"put\",\"resource\":\"/feeds/", FEED_ID, "\",\"params\":{},\"headers\":{\"X-ApiKey\":\"", API_KEY, "\"},\"body\":{\"version\":\"1.0.0\",\"datastreams\" : [ {\"id\" : \"temperature\",\"current_value\" : \"", ctemp, "\"},{\"id\" : \"humidity\",\"current_value\" : \"", chum, "\"}]}}");
    printf("SP ");
    printf(sensing_payload);
    printf("\r\n");
    websocket_send_str(&s, sensing_payload);
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(environode_process, ev, data)
{
  PROCESS_BEGIN();
  printf("Environode Debug!!!\r\n");
  // static struct etimer ds18b20_timer;
  static struct etimer sensing_timer;
  etimer_set(&sensing_timer, 15*CLOCK_SECOND);
  // ds18b20_init();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sensing_timer));
    /* We must init I2C each time, because the module lose his state when enter PM2 */
    i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN, I2C_SCL_FAST_BUS_SPEED);
    read_temperature(ctemp); // temperature SHT21
    printf("SHT21 temperature value: %s\r\n", ctemp);
    read_humidity(chum); // humidity SHT21
    printf("SHT21 humidity value: %s\r\n", chum);
    if(isReady == 0) {
      printf("********* isReady OK, Websocket PROCESS STARTED *********\r\n");
      process_start(&websocket_example_process, NULL);
    }
    isReady = 1;
    etimer_restart(&sensing_timer);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

// SOMETIMES CONNECTION IS TIMEDOUT WHILE SENDING, NEED TO ENSURE CONNECTION UP WHILE SENDING