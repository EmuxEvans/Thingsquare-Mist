#ifndef CONTIKI_CONF_H_
#define CONTIKI_CONF_H_

#include <stdint.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
/* Compiler configuration and platform-specific type definitions */
#define CLOCK_CONF_SECOND 128

/* Compiler configurations */
#define CCIF
#define CLIF

/* Platform typedefs */
typedef uint32_t clock_time_t;
typedef uint32_t uip_stats_t;

/*
 * rtimer.h typedefs rtimer_clock_t as unsigned short. We need to define
 * RTIMER_CLOCK_LT to override this
 */
typedef uint32_t rtimer_clock_t;
#define RTIMER_CLOCK_LT(a,b)     ((int32_t)((a)-(b)) < 0)
/*---------------------------------------------------------------------------*/
/* Configure the USB 'core' */
#define CTRL_EP_SIZE                8
#define USB_EP1_SIZE               32
#define USB_EP2_SIZE               64
#define USB_EP3_SIZE               64
#define USB_ARCH_WRITE_NOTIFY       0

/* Configure arch-specific USB */
#ifndef USB_ARCH_CONF_DMA
#define USB_ARCH_CONF_DMA           1
#endif
/*---------------------------------------------------------------------------*/
/* Energest Module */
#ifndef ENERGEST_CONF_ON
#define ENERGEST_CONF_ON            0
#endif

/* Verbose Startup? */
#ifndef STARTUP_CONF_VERBOSE
#define STARTUP_CONF_VERBOSE        1
#endif
/*---------------------------------------------------------------------------*/
/* uDMA Configuration and channel allocations */
#define USB_ARCH_CONF_RX_DMA_CHAN   0
#define USB_ARCH_CONF_TX_DMA_CHAN   1
#define CC2538_RF_CONF_TX_DMA_CHAN  2
#define CC2538_RF_CONF_RX_DMA_CHAN  3
#define UDMA_CONF_MAX_CHANNEL       CC2538_RF_CONF_RX_DMA_CHAN
/*---------------------------------------------------------------------------*/
/** Character I/O Configuration */
#ifndef UART_CONF_ENABLE
#define UART_CONF_ENABLE            1
#endif

#ifndef UART_CONF_BAUD_RATE
#define UART_CONF_BAUD_RATE    115200
#endif

#ifndef SLIP_ARCH_CONF_USB
#define SLIP_ARCH_CONF_USB          0 /** SLIP over UART by default */
#endif
#ifndef CC2538_RF_CONF_SNIFFER_USB
#define CC2538_RF_CONF_SNIFFER_USB  0 /** Sniffer out over UART by default */
#endif
#ifndef DBG_CONF_USB
#define DBG_CONF_USB                0 /** All debugging over UART by default */
#endif

/* Turn off example-provided putchars */
#define SLIP_BRIDGE_CONF_NO_PUTCHAR 1
#define SLIP_RADIO_CONF_NO_PUTCHAR  1

/*
 * Determine whether we need SLIP
 * This will keep working while UIP_FALLBACK_INTERFACE and CMD_CONF_OUTPUT
 * keep using SLIP
 */
#if defined (UIP_FALLBACK_INTERFACE) || defined (CMD_CONF_OUTPUT)
#define SLIP_ARCH_CONF_ENABLED      1
#endif

/** When set, the radio turns off address filtering and sends all captured
 * frames down a peripheral (UART or USB, depending on the value of
 * CC2538_RF_CONF_SNIFFER_USB) */
#ifndef CC2538_RF_CONF_SNIFFER
#define CC2538_RF_CONF_SNIFFER      0
#endif

/* Turn off RF RX DMA transfers if we are a sniffer */
#if CC2538_RF_CONF_SNIFFER
#undef CC2538_RF_CONF_RX_USE_DMA
#define CC2538_RF_CONF_RX_USE_DMA   0
#endif

/*
 * Define this as 1 to build a headless node. The UART will not be initialised
 * its clock will be gated, offering some energy savings. The USB will not be
 * initialised either
 */
#ifndef CC2538_CONF_QUIET
#define CC2538_CONF_QUIET           0
#endif

/* CC2538_CONF_QUIET is hard and overrides all other related defines */
#if CC2538_CONF_QUIET
#undef USB_SERIAL_CONF_ENABLE
#define USB_SERIAL_CONF_ENABLE      0

#undef UART_CONF_ENABLE
#define UART_CONF_ENABLE            0

#undef STARTUP_CONF_VERBOSE
#define STARTUP_CONF_VERBOSE        0

/* Little sanity check: We can't have quiet sniffers */
#if CC2538_RF_CONF_SNIFFER
#error "CC2538_RF_CONF_SNIFFER == 1 and CC2538_CONF_QUIET == 1"
#error "These values are conflicting. Please set either to 0"
#endif
#endif /* CC2538_CONF_QUIET */

/* Enable the USB core only if we need it */
#ifndef USB_SERIAL_CONF_ENABLE
#define USB_SERIAL_CONF_ENABLE \
  ((SLIP_ARCH_CONF_USB & SLIP_ARCH_CONF_ENABLED) | \
   DBG_CONF_USB | \
   (CC2538_RF_CONF_SNIFFER & CC2538_RF_CONF_SNIFFER_USB))
#endif

/* If debugging and SLIP use the same peripheral, set this define to 1 */
#if SLIP_ARCH_CONF_ENABLED
#define DBG_CONF_SLIP_MUX (SLIP_ARCH_CONF_USB==DBG_CONF_USB)
#endif
/*---------------------------------------------------------------------------*/
/* board.h assumes that basic configuration is done */
#include "board.h"
/*---------------------------------------------------------------------------*/
/* Network Stack */
#if UIP_CONF_IPV6
#define NETSTACK_CONF_NETWORK sicslowpan_driver
#else
#define NETSTACK_CONF_NETWORK rime_driver
#endif

#ifndef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     csma_driver
#endif

#ifndef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     contikimac_driver
#endif

/* Configure NullRDC for when it's selected */
#define NULLRDC_802154_AUTOACK                  1
#define NULLRDC_802154_AUTOACK_HW               1

/* Configure ContikiMAC for when it's selected */
#define CONTIKIMAC_CONF_WITH_CONTIKIMAC_HEADER  0
#define WITH_PHASE_OPTIMIZATION                 0
#define WITH_FAST_SLEEP                         1

#ifndef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE    8
#endif

#define NETSTACK_CONF_FRAMER  framer_802154
#define NETSTACK_CONF_RADIO   cc2538_rf_driver
/*---------------------------------------------------------------------------*/
/**
 * \name IEEE address configuration
 *
 * Used to generate our RIME & IPv6 address
 * @{
 */
/**
 * \brief Location of the IEEE address
 * 0 => Read from InfoPage,
 * 1 => Use a hardcoded address, configured by IEEE_ADDR_CONF_ADDRESS
 */
#ifndef IEEE_ADDR_CONF_HARDCODED
#define IEEE_ADDR_CONF_HARDCODED             0
#endif

#ifndef IEEE_ADDR_CONF_IN_FLASH
#define IEEE_ADDR_CONF_IN_FLASH              0
#endif

/**
 * \brief The hardcoded IEEE address to be used when IEEE_ADDR_CONF_HARDCODED
 * is defined as 1
 */
#ifndef IEEE_ADDR_CONF_ADDRESS
#define IEEE_ADDR_CONF_ADDRESS { 0x00, 0x12, 0x4B, 0x00, 0x89, 0xAB, 0xCD, 0xEF }
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/* RF Config */
// Defaults to 0xABCD in /net/mac/frame-802154.h
//#define IEEE802154_CONF_PANID          	    0xABCD

#ifndef CC2538_RF_CONF_CHANNEL
#define CC2538_RF_CONF_CHANNEL              26
#endif /* CC2538_RF_CONF_CHANNEL */

#ifndef CC2538_RF_CONF_AUTOACK
#define CC2538_RF_CONF_AUTOACK               1
#endif /* CC2538_CONF_AUTOACK */

/* RF/DMA Config */
#ifndef CC2538_RF_CONF_TX_USE_DMA
#define CC2538_RF_CONF_TX_USE_DMA            1
#endif

#ifndef CC2538_RF_CONF_RX_USE_DMA
#define CC2538_RF_CONF_RX_USE_DMA            1
#endif
/*---------------------------------------------------------------------------*/
/* IPv6, RIME and network buffer configuration */
#if UIP_CONF_IPV6
/* Addresses, Sizes and Interfaces */
/* 8-byte addresses here, 2 otherwise */
#define RIMEADDR_CONF_SIZE                   8
#define UIP_CONF_LL_802154                   1
#define UIP_CONF_LLH_LEN                     0
#define UIP_CONF_NETIF_MAX_ADDRESSES         3

/* TCP, UDP, ICMP */
#ifndef UIP_CONF_TCP
#define UIP_CONF_TCP                         1
#endif
#define UIP_CONF_TCP_MSS                    48
#define UIP_CONF_UDP                         1
#define UIP_CONF_UDP_CHECKSUMS               1
#define UIP_CONF_ICMP6                       1

/* ND and Routing */
#ifndef UIP_CONF_ROUTER
#define UIP_CONF_ROUTER                      1
#endif

#define UIP_CONF_IPV6_RPL                    1
#define UIP_CONF_ND6_SEND_RA                 0
#define UIP_CONF_IP_FORWARD                  0
#define RPL_CONF_STATS                       0
#define RPL_CONF_MAX_DAG_ENTRIES             1
#ifndef RPL_CONF_OF
#define RPL_CONF_OF rpl_mrhof
#endif

#define UIP_CONF_ND6_REACHABLE_TIME     600000
#define UIP_CONF_ND6_RETRANS_TIMER       10000

#ifndef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS                20
#endif
#ifndef UIP_CONF_DS6_ROUTE_NBU
#define UIP_CONF_DS6_ROUTE_NBU               4 /** Handle n Routes */
#endif

/* uIP */
#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE              1300
#endif

#define UIP_CONF_IPV6_QUEUE_PKT              0
#define UIP_CONF_IPV6_CHECKS                 1
#define UIP_CONF_IPV6_REASSEMBLY             0
#define UIP_CONF_MAX_LISTENPORTS             8

/* 6lowpan */
#define SICSLOWPAN_CONF_COMPRESSION          SICSLOWPAN_COMPRESSION_HC06
#define SICSLOWPAN_CONF_COMPRESSION_THRESHOLD 63
#ifndef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG                 1
#endif
#define SICSLOWPAN_CONF_MAXAGE               8

/* Define our IPv6 prefixes/contexts here */
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS    1
#define SICSLOWPAN_CONF_ADDR_CONTEXT_0 { \
  addr_contexts[0].prefix[0] = 0xaa; \
  addr_contexts[0].prefix[1] = 0xaa; \
}

#define MAC_CONF_CHANNEL_CHECK_RATE          8

#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM                    8
#endif
/*---------------------------------------------------------------------------*/
#else /* UIP_CONF_IPV6 */
/* Network setup for non-IPv6 (rime). */
#define UIP_CONF_IP_FORWARD                  1

#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE               108
#endif

#define RIME_CONF_NO_POLITE_ANNOUCEMENTS     0

#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM                    8
#endif

#endif /* UIP_CONF_IPV6 */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Include Project Specific conf */
#ifdef PROJECT_CONF_H
#include PROJECT_CONF_H
#endif /* PROJECT_CONF_H */

#ifdef PLATFORM_CONF
#include PLATFORM_CONF
#endif /* PLATFORM_CONF */


#endif /* CONTIKI_CONF_H_ */
