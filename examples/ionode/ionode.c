#include "mist.h"
#include "websocket.h"
#include "dev/gpio.h"
#include "dev/adc.h"

static struct websocket s;

static void callback(struct websocket *s, websocket_result r,
                     uint8_t *data, uint16_t datalen);

#define RECONNECT_INTERVAL 10 * CLOCK_SECOND
static struct ctimer reconnect_timer;

/*---------------------------------------------------------------------------*/
PROCESS(ionode_process, "Digital output and input");
AUTOSTART_PROCESSES(&ionode_process);
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
