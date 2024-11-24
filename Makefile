CONTIKI_PROJECT = error_handling
all: $(CONTIKI_PROJECT)

# Include the Contiki build system
CONTIKI = ../contiki-2.7
include $(CONTIKI)/Makefile.include
