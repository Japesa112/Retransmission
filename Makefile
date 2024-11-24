# Path to your Contiki installation
CONTIKI = ../contiki-2.7

# Define the name of the executable
TARGET = udp_client_retransmission

# Add source files to be compiled
PROJECT_SOURCEFILES += udp_client_retransmission.c

# Include Contiki's Makefile to handle the build process
include $(CONTIKI)/Makefile.include
