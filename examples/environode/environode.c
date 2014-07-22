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

/*---------------------------------------------------------------------------*/
PROCESS(environode_process, "Environode Sensing");
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
    char buf[] = "012345678";
    static int count;
    buf[0] = (count % 9) + '0';
    count++;
    websocket_send_str(&s, buf);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS(i2c_example, "Example of I2C using TCN75 / SHT21");
AUTOSTART_PROCESSES(&i2c_example);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(i2c_example, ev, data)
{
	PROCESS_BEGIN();

	while(1) {
		etimer_set(&timer, CLOCK_CONF_SECOND * MEASURE_INTERVAL_TIME_SECOND);
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
		/* We must init I2C each time, because the module lose his state when enter PM2 */
		i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN, I2C_SCL_NORMAL_BUS_SPEED);
		#if TCN75_ENABLE
			if(read_temp_TCN75(&temp) == I2C_MASTER_ERR_NONE) {
				temp = temp>>7;
				printf("\n%d.%d", temp/2, (temp%2)*5);
			} else {
				printf("\nError");
			}
		#endif //TCN75_ENABLE
		#if SHT21_ENABLE
			if(read_temp_SHT21(&temp) == I2C_MASTER_ERR_NONE) {
				printf("\n");
				print_float(((((float)temp)/65536)*175.72-46.85), SHT21_DECIMAL_PRECISION);
			} else {
				printf("\nError");
			}
		#endif //SHT21_ENABLE
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
