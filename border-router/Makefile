LIBMC1322X = ../../libmc1322x
CONTIKI = ../../contiki

CONTIKI_PROJECT = border-router
TARGET = econotag
CLEAN = *.d

all: $(CONTIKI_PROJECT) 

WITH_UIP6=1
UIP_CONF_IPV6=1
CFLAGS += -DUIP_CONF_IPV6_RPL

#linker optimizations
SMALL=1

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
PROJECT_SOURCEFILES += slip-bridge.c

#Simple built-in webserver is the default.
#Override with make WITH_WEBSERVER=0 for no webserver.
#WITH_WEBSERVER=webserver-name will use /apps/webserver-name if it can be
#found in the /apps, /platform/$(TARGET)/apps/, or current directory (in that order).
# WITH_WEBSERVER=webserver for /apps/webserver
# WITH_WEBSERVER=raven-webserver for /platform/avr-raven/apps/raven-webserver/
#make clean before changing webservers!

#Note /apps/webserver contains a 2500 byte style sheet which is a severe test
#of the slip connection. Large MSS together with low baud rates without flow
#control will overrun the transmit buffer when the style sheet is requested.

WITH_WEBSERVER=1
ifeq ($(WITH_WEBSERVER),1)
CFLAGS += -DWEBSERVER=1
PROJECT_SOURCEFILES += httpd-simple.c
else ifneq ($(WITH_WEBSERVER), 0)
APPS += $(WITH_WEBSERVER)
CFLAGS += -DWEBSERVER=2
endif
CFLAGS += -DRADIODEBUGLED=1

ifeq ($(PREFIX),)
 PREFIX = aaaa::1/64
endif

clear:
	-rm d03_e_econotag.bin
	-rm d03_e_econotag.txt
	-rm d03_e_econotag.pbm
	sudo $(LIBMC1322X)/tools/ftditools/bbmc -l $(TARGET) -i 0 erase

upload:
	../blaster/blaster $(CONTIKI_PROJECT).cfg \
	< $(CONTIKI_PROJECT)_$(TARGET).bin \
	> d03_e_econotag.bin
	
	$(LIBMC1322X)/tools/mc1322x-load \
	-f $(LIBMC1322X)/tests/flasher_$(TARGET).bin -s d03_e_econotag.bin \
	-c 'sudo $(LIBMC1322X)/tools/ftditools/bbmc -l $(TARGET) -i 0 reset' \
	-t /dev/ttyUSB1 -l -e

$(CONTIKI)/tools/tunslip6:	$(CONTIKI)/tools/tunslip6.c
	(cd $(CONTIKI)/tools && $(MAKE) tunslip6)

border: $(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -s /dev/ttyUSB1 $(PREFIX)

border-cooja: $(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -a 127.0.0.1 $(PREFIX)

include $(CONTIKI)/Makefile.include
