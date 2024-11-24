#include "contiki.h"
#include "contiki-net.h"
#include <stdio.h>
#include <string.h>

#define SERVER_IP "fd00::1"     // Server IPv6 address
#define CLIENT_PORT 8765        // Client port number
#define SERVER_PORT 5678        // Server port number
#define RETRANSMISSION_TIMEOUT (CLOCK_SECOND * 5)

static struct uip_udp_conn *client_conn;   // UDP connection
static char payload[64];                   // Payload to send
static int retransmission_count = 0;       // Retransmission attempts
static int ack_received = 0;               // Flag for ACK reception

PROCESS(udp_client_retransmission_process, "UDP Client with Retransmission");
AUTOSTART_PROCESSES(&udp_client_retransmission_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_retransmission_process, ev, data) {
  static struct etimer timer;

  PROCESS_BEGIN();

  // Create a new UDP connection
  uip_ip6addr_t server_ipaddr;
  uip_ip6addr(&server_ipaddr, 0xfd00, 0, 0, 0, 0, 0, 0, 1); // fd00::1
  client_conn = udp_new(&server_ipaddr, UIP_HTONS(SERVER_PORT), NULL);
  if (!client_conn) {
    printf("Failed to create UDP connection\n");
    PROCESS_EXIT();
  }
  udp_bind(client_conn, UIP_HTONS(CLIENT_PORT));
  printf("UDP client started. Sending packets to [%s]:%u\n", SERVER_IP, SERVER_PORT);

  // Start periodic packet sending
  etimer_set(&timer, CLOCK_SECOND * 5);

  while (1) {
    // Wait for the timer event
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

    // Create the payload
    sprintf(payload, "Hello %d", retransmission_count);

    // Reset the ACK flag
    ack_received = 0;

    // Send the payload
    printf("Sending packet: %s\n", payload);
    uip_udp_packet_sendto(client_conn, payload, strlen(payload), &client_conn->ripaddr, UIP_HTONS(SERVER_PORT));

    // Wait for an acknowledgment
    etimer_set(&timer, RETRANSMISSION_TIMEOUT);
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event || etimer_expired(&timer));

    // Handle acknowledgment
    if (ev == tcpip_event && uip_newdata()) {
      char *received_data = (char *)uip_appdata;
      received_data[uip_datalen()] = '\0'; // Null-terminate the received data

      if (strcmp(received_data, "ACK") == 0) {
        printf("ACK received for packet: %s\n", payload);
        ack_received = 1;
        retransmission_count = 0; // Reset retransmission count
      }
    }

    // Handle retransmission
    if (!ack_received) {
      printf("No ACK received. Retransmitting...\n");
      retransmission_count++;
      if (retransmission_count >= 3) {
        printf("Failed after 3 retransmissions. Giving up on this packet.\n");
        retransmission_count = 0; // Reset retransmission count
      }
    }

    // Reset the timer for the next packet
    etimer_reset(&timer);
  }

  PROCESS_END();
}
