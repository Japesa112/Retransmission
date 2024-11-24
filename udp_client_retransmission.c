#include "contiki.h"
#include "net/ipv6/simple-udp.h"
#include <stdio.h>
#include <string.h>

#define UDP_PORT 1234
#define MAX_RETRANSMISSIONS 3

static struct simple_udp_connection udp_conn;
static char payload[] = "Hello, Server!";

PROCESS(udp_client_process, "UDP Client with Retransmission");
AUTOSTART_PROCESSES(&udp_client_process);

PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer timer;
  static int retransmissions = 0;
  static int ack_received = 0;

  PROCESS_BEGIN();

  // Setup UDP connection
  simple_udp_register(&udp_conn, UDP_PORT, NULL, UDP_PORT, NULL);

  while(1) {
    etimer_set(&timer, CLOCK_SECOND * 2);
    printf("Sending packet: %s\n", payload);
    simple_udp_send(&udp_conn, payload, strlen(payload));
    
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer) || ack_received);

    if (!ack_received) {
      retransmissions++;
      if (retransmissions >= MAX_RETRANSMISSIONS) {
        printf("Max retransmissions reached. Dropping packet.\n");
        retransmissions = 0;
      } else {
        printf("Retransmitting...\n");
        etimer_reset(&timer);
      }
    } else {
      printf("ACK received!\n");
      retransmissions = 0;
    }
  }

  PROCESS_END();
}
