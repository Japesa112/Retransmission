#include "contiki.h"
#include "net/netstack.h"
#include "net/packetbuf.h"

PROCESS(interference_process, "Interference Generator");
AUTOSTART_PROCESSES(&interference_process);

#define MAX_RETRIES 3 // Maximum number of retransmissions
#define RETRY_DELAY CLOCK_SECOND / 20 // Delay between retries (50ms)

PROCESS_THREAD(interference_process, ev, data) {
  static struct etimer timer;
  static uint8_t retries;

  PROCESS_BEGIN();

  // Generate interference every 100ms
  etimer_set(&timer, CLOCK_SECOND / 10);
  retries = 0;

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

    // Dummy packet for interference
    packetbuf_copyfrom("NOISE", 5); 

    // Attempt to send the interference
    while (NETSTACK_MAC.send(NULL, NULL) == 0 && retries < MAX_RETRIES) {
      // If send failed, wait a little and retry
      retries++;
      etimer_set(&timer, RETRY_DELAY);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    }

    // Reset retry counter for the next transmission
    retries = 0;

    // Reset the timer for the next interference event
    etimer_reset(&timer);
  }

  PROCESS_END();
}
