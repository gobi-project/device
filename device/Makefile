LIBMC1322X = ../../libmc1322x
CONTIKI = ../../contiki

CONTIKI_PROJECT = d13
TARGET = econotag
CLEAN = *.d d*_e_$(TARGET).bin d*_e_$(TARGET).txt d*_e_$(TARGET).pbm

all: $(CONTIKI_PROJECT) copy

WITH_UIP6=1
UIP_CONF_IPV6=1

CFLAGS += -DWITH_DTLS=0
CFLAGS += -DREST=coap_rest_implementation
CFLAGS += -DUIP_CONF_TCP=0
CFLAGS += -DSTACKMONITOR=1  # Initialiserung um den maximal benötigten Stack ermitteln zu können
CFLAGS += -DHEAPMONITOR=1   # Initialiserung um den maximal benötigten Heap ermitteln zu können
CFLAGS += -DRADIODEBUGLED=1 # Aktiviert die rote LED bei Datentransfer über 802.15.4
CFLAGS += -DOWN_SENSORS_DEFINITION
CFLAGS += -DREST_MAX_CHUNK_SIZE=48

# CFLAGS += -Wcast-align -Wall -Wstrict-prototypes -Wextra
# http://www.gnu.org/software/gsl/manual/html_node/GCC-warning-options-for-numerical-programs.html

LDFLAGS += "-Wl,-gc-sections"

# zusätzliche Bibliotheken aus $(CONTIKI)/apps
APPS += er-dtls
APPS += rest-engine
APPS += er-coap
APPS += flash-store
APPS += aes
APPS += ecc

# der eigentliche Code (nicht benoetigt, wenn nur eine Datei...)
# Wird hier durch include der .c-Files in device.c gelöst
PROJECT_SOURCEFILES = 

copy:
#	cp $(CONTIKI_PROJECT)_$(TARGET).bin ../codtls-client/$(CONTIKI_PROJECT)_$(TARGET).bin

clear:
	sudo $(LIBMC1322X)/tools/ftditools/bbmc -l $(TARGET) -i 0 erase

upload:
ifndef MAC
	${info MAC not defined. Use 'make upload MAC=XX'}
else
	../blaster/blaster d$(MAC).cfg < d$(MAC)_$(TARGET).bin > \
	d$(MAC)_e_$(TARGET).bin
	
	$(LIBMC1322X)/tools/mc1322x-load \
	-f $(LIBMC1322X)/tests/flasher_$(TARGET).bin \
	-s d$(MAC)_e_$(TARGET).bin \
	-c 'sudo $(LIBMC1322X)/tools/ftditools/bbmc -l $(TARGET) -i 0 reset' \
	-t /dev/ttyUSB1 -l -e \
	
	sudo $(LIBMC1322X)/tools/ftditools/bbmc -l $(TARGET) -i 0 reset
endif

listen:
	while true; do cat /dev/ttyUSB1; done

list:
	cat devices.txt

include $(CONTIKI)/Makefile.include
