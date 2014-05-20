LIBMC1322X = ../../libmc1322x
CONTIKI = ../../contiki

CONTIKI_PROJECT = d19 d28 da0 da1 da2 da3 da4 da5 da6 da7 da8 da9
TARGET = econotag
<<<<<<< HEAD
CLEAN = *.d
=======
CLEAN = *.d d*_e_$(TARGET).bin d*_e_$(TARGET).txt d*_e_$(TARGET).pbm
>>>>>>> fc75da2be3b39cd55ef0edb3a95e02dfb00480f2

all: $(CONTIKI_PROJECT) blast

WITH_UIP6=1
UIP_CONF_IPV6=1

CFLAGS += -DWITH_DTLS=1
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

blast:
	../blaster/blaster $(shell echo $(CONTIKI_PROJECT).cfg | sed "s/ /.cfg /g")

clear:
	sudo $(LIBMC1322X)/tools/ftditools/bbmc -l $(TARGET) -i 0 erase

upload:
ifndef MAC
	${info MAC not defined. Use 'make upload MAC=XX'}
else
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
