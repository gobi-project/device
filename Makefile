LIBMC1322X = ../libmc1322x
CONTIKI = ../contiki

CONTIKI_PROJECT = d_button d_socket d_temperature d_led d_rgb d_lux d_dht d_test
DEVICES = db1 db2 da6 da7 da8 d28 d29 d25 d13 d30 d31 d19
TARGET = econotag
CLEAN = *.d d*_$(TARGET).bin d*_$(TARGET).txt d*_$(TARGET).pbm cfg-parser/cfg_parser.o cfg-parser/cfg_parser

all: $(CONTIKI_PROJECT) blast

WITH_UIP6=1
UIP_CONF_IPV6=1

CFLAGS += -DWITH_DTLS=1
CFLAGS += -DHELLO_REQUEST=1
CFLAGS += -DREST=coap_rest_implementation
CFLAGS += -DUIP_CONF_TCP=0
CFLAGS += -DSTACKMONITOR=0  # Initialiserung um den maximal benötigten Stack ermitteln zu können
CFLAGS += -DHEAPMONITOR=0   # Initialiserung um den maximal benötigten Heap ermitteln zu können
CFLAGS += -DRADIODEBUGLED=1 # Aktiviert die rote LED bei Datentransfer über 802.15.4
CFLAGS += -DOWN_SENSORS_DEFINITION
CFLAGS += -DREST_MAX_CHUNK_SIZE=48
CFLAGS += -DFLASH_CONF_B1=658
CFLAGS += -DFLASH_CONF_B2=801

# CFLAGS += -Wcast-align -Wall -Wstrict-prototypes -Wextra
# http://www.gnu.org/software/gsl/manual/html_node/GCC-warning-options-for-numerical-programs.html

LDFLAGS += "-Wl,-gc-sections"

# zusätzliche Bibliotheken aus $(CONTIKI)/apps
APPS += rest-engine
APPS += er-coap
APPS += aes
APPS += ecc
APPS += flash
APPS += er-dtls

# der eigentliche Code (nicht benötigt, wenn nur eine Datei...)
# Wird hier durch include der .c-Files in device.c gelöst
PROJECT_SOURCEFILES = 

blast: $(CONTIKI)/tools/blaster/blaster
	$(CONTIKI)/tools/blaster/blaster $(shell echo $(DEVICES).cfg | sed "s/ /.cfg /g")

$(CONTIKI)/tools/blaster/blaster:	$(CONTIKI)/tools/blaster/blaster.c
	(cd $(CONTIKI)/tools/blaster && $(MAKE))

clear:
	sudo $(LIBMC1322X)/tools/ftditools/bbmc -l $(TARGET) -i 0 erase

upload:
ifndef MAC
	${info MAC not defined. Use 'make upload MAC=XX'}
else
	$(LIBMC1322X)/tools/mc1322x-load \
	-f $(LIBMC1322X)/tests/flasher_$(TARGET).bin \
	-s d$(MAC)_$(TARGET).bin \
	-c 'sudo $(LIBMC1322X)/tools/ftditools/bbmc -l $(TARGET) -i 0 reset' \
	-t /dev/ttyUSB1 -l -e \
	
	sudo $(LIBMC1322X)/tools/ftditools/bbmc -l $(TARGET) -i 0 reset
endif

listen:
	while true; do cat /dev/ttyUSB1; done

list: cfg-parser/cfg_parser
	cfg-parser/cfg_parser .

cfg-parser/cfg_parser: cfg-parser/cfg_parser.c
	make -C cfg-parser

include $(CONTIKI)/Makefile.include
