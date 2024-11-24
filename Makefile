CONTIKI = ../contiki-2.7

TARGET = sky

all: udp_client_retransmission

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
