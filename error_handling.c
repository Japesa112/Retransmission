#include "contiki.h"
#include "net/rime/rime.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_RETRANSMISSIONS 3
#define ERROR_PROBABILITY 0.1 // 10% chance of error

static struct unicast_conn uc;
static int retransmissions;

PROCESS(error_handling_process, "Error Handling Process");
AUTOSTART_PROCESSES(&error_handling_process);

// Function to simulate bit errors
int introduce_bit_error(int message) {
    if ((rand() / (double)RAND_MAX) < ERROR_PROBABILITY) {
        // Introduce an error by flipping the least significant bit
        return message ^ 0x01;
    }
    return message;
}

// Unicast receive callback
static void recv_uc(struct unicast_conn *c, const linkaddr_t *from) {
    int received_message = *(int *)packetbuf_dataptr();
    printf("Received message: %d from %d.%d\n", received_message, from->u8[0], from->u8[1]);

    // Simulate error detection (for simplicity, assume even number is valid)
    if (received_message % 2 == 0) {
        printf("Message received correctly.\n");
        retransmissions = 0; // Reset retransmissions
    } else {
        printf("Error detected! Retransmitting...\n");
        if (retransmissions < MAX_RETRANSMISSIONS) {
            retransmissions++;
            packetbuf_copyfrom(&received_message, sizeof(received_message));
            unicast_send(&uc, from);
        } else {
            printf("Max retransmissions reached. Giving up.\n");
        }
    }
}

// Unicast send callback
static void sent_uc(struct unicast_conn *c, const linkaddr_t *to) {
    printf("Message sent to %d.%d\n", to->u8[0], to->u8[1]);
}

PROCESS_THREAD(error_handling_process, ev, data) {
    static struct etimer timer;
    static int message;

    unicast_open(&uc, 146, NULL);
    srand(clock_time()); // Seed for random number generation

    while (1) {
        etimer_set(&timer, CLOCK_SECOND * 2);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

        // Prepare message
        message = rand() % 100; // Random message
        message = introduce_bit_error(message); // Introduce possible error

        packetbuf_copyfrom(&message, sizeof(message));
        unicast_send(&uc, &linkaddr_node_addr); // Send to self for testing
        printf("Sent message: %d\n", message);
    }
}