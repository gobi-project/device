CONTIKI_PROJECT = device

CONTIKI = ../../contiki
LIBMC1322X = ../../libmc1322x
TARGET = econotag
all: $(CONTIKI_PROJECT) copy

WITH_UIP6=1
UIP_CONF_IPV6=1
WITH_DTLS=1
WITH_COAP=13

CFLAGS += -DWITH_DTLS=1
CFLAGS += -DWITH_COAP=13
CFLAGS += -DREST=coap_rest_implementation
CFLAGS += -DUIP_CONF_TCP=0
#??? CFLAGS += -DDEBUG_PRINT=1
CFLAGS += -DSTACKMONITOR=1  # Initialiserung um den maximal benötigten Stack ermitteln zu können
CFLAGS += -DHEAPMONITOR=1   # Initialiserung um den maximal benötigten Heap ermitteln zu können
CFLAGS += -DRADIODEBUGLED=1 # Aktiviert die rote LED bei Datentransfer über 802.15.4

# CFLAGS += -Wcast-align -Wall -Wstrict-prototypes -Wextra
# http://www.gnu.org/software/gsl/manual/html_node/GCC-warning-options-for-numerical-programs.html

LDFLAGS += "-Wl,-gc-sections"

# zusätzliche Bibliotheken aus $(CONTIKI)/apps
APPS += er-dtls-13
APPS += er-coap-13
APPS += erbium
APPS += flash-store
APPS += aes
APPS += ecc

# der eigentliche Code (nicht benoetigt, wenn nur eine Datei...)
PROJECT_SOURCEFILES = attributes.c flasher.c

copy:
#	cp $(CONTIKI_PROJECT)_$(TARGET).bin ../client/$(CONTIKI_PROJECT)_$(TARGET).bin

clear:
	-rm $(CONTIKI_PROJECT)_e_$(TARGET).bin
	-rm $(CONTIKI_PROJECT)_e_$(TARGET).txt
	-rm qr-code.pbm
	sudo $(LIBMC1322X)/tools/ftditools/bbmc -l $(TARGET) -i 0 erase

upload:
	../blaster/blaster 19 -t < $(CONTIKI_PROJECT)_$(TARGET).bin > \
	$(CONTIKI_PROJECT)_e_$(TARGET).bin 2> \
	$(CONTIKI_PROJECT)_e_$(TARGET).txt \
	
	$(LIBMC1322X)/tools/mc1322x-load \
	-f $(LIBMC1322X)/tests/flasher_$(TARGET).bin \
	-s $(CONTIKI_PROJECT)_e_$(TARGET).bin \
	-c 'sudo $(LIBMC1322X)/tools/ftditools/bbmc -l $(TARGET) -i 0 reset' \
	-t /dev/ttyUSB1 -l -e \
	
	sudo $(LIBMC1322X)/tools/ftditools/bbmc -l $(TARGET) -i 0 reset

listen:
	while true; do cat /dev/ttyUSB1; done

include $(CONTIKI)/Makefile.include
