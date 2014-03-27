LIBMC1322X = ../../libmc1322x
CONTIKI = ../../contiki

CONTIKI_PROJECT = device
TARGET = econotag

all: $(CONTIKI_PROJECT) copy

WITH_UIP6=1
UIP_CONF_IPV6=1

CFLAGS += -DWITH_DTLS=1
CFLAGS += -DREST=coap_rest_implementation
CFLAGS += -DUIP_CONF_TCP=0
# ??? CFLAGS += -DDEBUG_PRINT=1
CFLAGS += -DSTACKMONITOR=1  # Initialiserung um den maximal benötigten Stack ermitteln zu können
CFLAGS += -DHEAPMONITOR=1   # Initialiserung um den maximal benötigten Heap ermitteln zu können
CFLAGS += -DRADIODEBUGLED=1 # Aktiviert die rote LED bei Datentransfer über 802.15.4

#sensors
CFLAGS += -DLEDB=0        # binäre LED (GPIO 43)
CFLAGS += -DLEDD=0		  # dimmbare LED (TMR2)
#CFLAGS += -DBUTTON2=0     # Button 2 auf dem Econotag
CFLAGS += -DBUTTON_EXT=0  # externer Button (kbi6)
CFLAGS += -DTMP=0         # Temperatur-Sensor (i2c)
CFLAGS += -DLUX=0         # Lux-Sensor (i2c)
CFLAGS += -DDHT=0         # digitaler Luftfeuchtigkeit- und Temperatur-Sensor (GPIO 42)
CFLAGS += -DPOTI=0   	  # Potentiometer (ADC0)
CFLAGS += -DRGB=1         # RGB Licht per DMX (GPIO 42)

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
	cp $(CONTIKI_PROJECT)_$(TARGET).bin ../codtls-client/$(CONTIKI_PROJECT)_$(TARGET).bin

clear:
	-rm $(CONTIKI_PROJECT)_e_$(TARGET).bin
	-rm $(CONTIKI_PROJECT)_e_$(TARGET).txt
	-rm qr-code.pbm
	sudo $(LIBMC1322X)/tools/ftditools/bbmc -l $(TARGET) -i 0 erase

upload:
	../blaster/blaster 14 -t < $(CONTIKI_PROJECT)_$(TARGET).bin > \
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
