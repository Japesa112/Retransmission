#include "contiki.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "sys/etimer.h"
#include "sys/clock.h"

PROCESS(interference_process, "Interference Generator");
AUTOSTART_PROCESSES(&interference_process);

#define MAX_RETRIES 3          // Maximum number of retransmissions
#define RETRY_DELAY CLOCK_SECOND / 20 // Delay between retries (50ms)
#define SUCCESS_PROBABILITY 0.7  // 70% chance of transmission success

// Define RAND_MAX manually for Contiki
#define RAND_MAX 32767  // The maximum value that random_rand() can return

static struct etimer timer;
static uint8_t retries = 0;

PROCESS_THREAD(interference_process, ev, data) {
  PROCESS_BEGIN();

  // Generate interference every 100ms
  etimer_set(&timer, CLOCK_SECOND / 10);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

    // Dummy packet for interference
    packetbuf_copyfrom("NOISE", 5); 

    // Attempt to send the interference
    while (retries < MAX_RETRIES) {
      NETSTACK_MAC.send(NULL, NULL);  // Send the packet
      // Simulate waiting for some response or a condition to check success
      etimer_set(&timer, RETRY_DELAY);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

      // Simulate whether the transmission was successful based on a probability
      if (random_rand() < (SUCCESS_PROBABILITY * RAND_MAX)) {
        // Transmission was successful
        printf("Transmission successful\n");
        break;  // Exit retransmission loop on success
      }
      
      retries++;  // Increment retries
      printf("Transmission failed, retrying...\n");
    }

    // Reset retry counter for the next transmission
    retries = 0;

    // Reset the timer for the next interference event
    etimer_reset(&timer);
  }

  PROCESS_END();
}
