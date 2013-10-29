LIBMC1322X = ../../libmc1322x
CONTIKI = ../../contiki

CONTIKI_PROJECT = $(LIBMC1322X)/tests/rftest-rx
TARGET = econotag

all: upload

clear:
	sudo $(LIBMC1322X)/tools/ftditools/bbmc -l $(TARGET) erase

upload:
	$(LIBMC1322X)/tools/mc1322x-load \
	-f $(LIBMC1322X)/tests/flasher_$(TARGET).bin \
	-s $(CONTIKI_PROJECT)_$(TARGET).bin 0x1e000,0xB160B162,0x2800B160 \
	-c 'sudo $(LIBMC1322X)/tools/ftditools/bbmc -l $(TARGET) -i 0 reset' \
	-t /dev/ttyUSB1 -e \
	
	sudo $(LIBMC1322X)/tools/ftditools/bbmc -l $(TARGET) -i 0 reset

listen:
	$(LIBMC1322X)/tools/rftestrx2pcap.py /dev/ttyUSB3 26 | wireshark -k -i -

listen2:
	$(LIBMC1322X)/tools/rftestrx2pcap.py /dev/ttyUSB3 26 > cap.pcap
