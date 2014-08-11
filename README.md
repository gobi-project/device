device
======

Vorraussetzungen sind Contiki und libmc1322x, dessen Git-Repositorys auf der selben Ebene wie Device ausgecheckt sein müssen. Nur die Contiki-Version des
GOBI-Projekts unterstützt alle notwendigen Funktionen.

make baut und konfiguriert mit Hilfe von Contiki alle spezifizierten Endgeräte.

make upload MAC=XX läd mit Hilfe der libmc1322x die Firmware für das Endgerät mit der angegebenen MAC-Endummer auf den angeschlossenen Econotag.
