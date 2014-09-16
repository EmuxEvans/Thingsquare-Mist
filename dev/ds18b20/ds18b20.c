/*
 * Copyright (c) 2012, Jens Nielsen
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL JENS NIELSEN BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "contiki.h"
#include "mist.h"
#include "owhal.h" 
#include "ownet.h"
#include "owlink.h"
#include "ds18b20.h"
#include <string.h> /*for memset*/
#include <stdio.h>
 
#define DBG(...) printf(__VA_ARGS__)
// #define DBG(...)

typedef struct
{
    unsigned char serial[ 8 ];
    float lastTemp;
}device_t;

static device_t devices[ NUM_DEVICES ];

typedef enum state_e
{
    STATE_SCAN,
    STATE_CONVERT,
    STATE_WAIT_CONVERT,
    STATE_FETCH_TEMPS,
}state_t;

static state_t state;
static struct stimer convert_timer;
static void ds18b20_scan(void);
static void ds18b20_startConvert(void);
static void ds18b20_fetchTemp( unsigned char device );

void ds18b20_init()
{
  DBG("ds18b20_init\r\n");
  memset( devices, 0, sizeof(devices) );
  state = STATE_SCAN;  
}

void read_temperatures(char ** ctemps)
{  
  state = STATE_SCAN;  
  unsigned char fetchDevice = 0;
  DBG("fetchDevice: %d, state: %d\r\n", fetchDevice, state);
  while(fetchDevice < NUM_DEVICES ) {
    switch ( state )
    {
      case STATE_SCAN:
        DBG("STATE_SCAN\r\n");
        ds18b20_scan();
        state = STATE_CONVERT;
      break;

      case STATE_CONVERT:
        DBG("STATE_CONVERT\r\n");
        ds18b20_startConvert();
        stimer_set( &convert_timer, CLOCK_SECOND );
        state = STATE_WAIT_CONVERT;        
      break;

      case STATE_WAIT_CONVERT:
        if ( stimer_expired( &convert_timer ) ) {
          DBG("finish convert_timer\r\n");
          state = STATE_FETCH_TEMPS;
          stimer_restart( &convert_timer );
        }
        else {
          DBG("STATE_WAIT_CONVERT\r\n");
        }
      break;

      case STATE_FETCH_TEMPS:
        DBG("STATE_FETCH_TEMPS\r\n");
        ds18b20_fetchTemp( fetchDevice );
        DBG("Device %d temp value: %.2f\r\n", fetchDevice, devices[fetchDevice].lastTemp);
        sprintf(ctemps[fetchDevice], "%.2f",  devices[fetchDevice].lastTemp);
        fetchDevice++;
        if ( fetchDevice >= NUM_DEVICES )
          state = STATE_CONVERT;
      break;
    }
  }
}

static void ds18b20_scan()
{
  unsigned char found;
  unsigned char i;

  found = owFirst( 0, 1, 0 );
  if ( found )
  {
    DBG("ds18b20_scan found\r\n");
    owSerialNum( 0, devices[ 0 ].serial, 1 );
    // DBG("ds18b20_scan serial number: ");
    // int iz;
    // for (iz=0; iz<8; iz++){
    //   DBG("%d.", devices[ 0 ].serial[iz]);
    // }
    // DBG("\r\n");
  }

  for ( i = 1; found && i < NUM_DEVICES; i++ )
  {
    found = owNext( 0, 1, 0 );
    if ( found )
    {
      owSerialNum( 0, devices[ i ].serial, 1 );
    }
  }
}

static void ds18b20_startConvert( )
{
  int i;
  owTouchReset();
  // owWriteByte(0xCC);
  owWriteByte(0x55);
  for ( i = 0; i < 8; i++ ) owWriteByte(devices[ 0 ].serial[i]);
  owWriteByte(0x44);
  DBG("finish owWriteByte 44\r\n");
}

static void ds18b20_fetchTemp( unsigned char device )
{
  if ( device < NUM_DEVICES && devices[ device ].serial[ 0 ] != 0 )
  {
    unsigned char i;
    unsigned char data[10];

    /*address specified device*/
    owTouchReset();
    owWriteByte(0x55);
    for (i = 0; i < 8; i++)
    {
      owWriteByte(devices[ device ].serial[ i ]);
    }

    /*read the first two bytes of scratchpad*/
    owWriteByte(0xBE);

    for ( i = 0; i < 9; i++) {
      data[i] = owReadByte();
    }

    DBG("ds18b20_fetchTemp data: ");
    int iz;
    for (iz=0; iz<11; iz++){
      DBG("%d.", data[i]);
    }
    DBG("\r\n");

    DBG("ds18b20_fetchTemp lastTemp: %3.2f\r\n", ( (data[1] << 8) + data[0] )*0.0625);
    devices[ device ].lastTemp = ( (data[1] << 8) + data[0] )*0.0625;
  }
  else {
    DBG("device: %d, devices[ device ].serial[ 0 ]: %d\r\n", device, devices[ device ].serial[ 0 ] );
    DBG("ds18b20_fetchTemp failed\r\n");
  }
}

