#include "mist.h"
#include "websocket.h"
#include "dev/sht21/sht21.h"
#include "dev/ds18b20/ds18b20.h"
#include "dev/ds18b20/owhal.h"

static struct websocket s;

static void callback(struct websocket *s, websocket_result r,
                     uint8_t *data, uint16_t datalen);

#define RECONNECT_INTERVAL 10 * CLOCK_SECOND
static struct ctimer reconnect_timer;

char * ctemp;
char * chum;
char * ctemps;

/*---------------------------------------------------------------------------*/
PROCESS(environode_process, "Environode sensing and sending");
AUTOSTART_PROCESSES(&environode_process);
/*---------------------------------------------------------------------------*/
static void
reconnect_callback(void *ptr)
{
  websocket_open(&s, "ws://172.16.0.1:8080/",
                 "thingsquare mist", callback);
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
    printf("environode: Received data '%.*s' (len %d)\n", datalen,
           data, datalen);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(environode_process, ev, data)
{
  static struct etimer et;
  PROCESS_BEGIN();

  ctimer_set(&reconnect_timer, RECONNECT_INTERVAL, reconnect_callback, &s);

  while(1) {
    etimer_set(&et, CLOCK_SECOND / 8);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  
    ctemp = malloc(10 * sizeof(char));
    read_temperature(&ctemp); // temperature SHT21
    free(ctemp);

    /*
    chum = malloc(10 * sizeof(char));
    read_humidity(&chum); // humidity SHT21
    free(chum);
    */

    /*
    ctemps = malloc(NUM_DEVICES * sizeof(char*));
    for(int num_dev = 0; num_dev <NUM_DEVICES; num_dev++) {
      ctemps[num_dev] = malloc(10 * sizeof(char));
    }
    read_temperatures(ctemps); // temperatures DS18b20
    free(ctemps);
    */

    static int count;
    count++;
    websocket_send_str(&s, ctemp);
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
/**
 * @}
 */
