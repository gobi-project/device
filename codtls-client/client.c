#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <uuid/uuid.h>
#include <time.h>

#include "ip_list.h"
#include "ip_tools.h"
#include "border_com.h"
#include "node_com.h"

struct ip_list *liste = NULL;
uint8_t psk[16];

void printhex(uint8_t *d, uint8_t l) {
  int i;
  for (i = 0; i < l; i++) printf("%02X", d[i]);
}

int main(int argc, char *argv[]) {
    // TODO vor jedem Kompilieren muss derzeit der psk vom econotag gesetzt werden
    memcpy(psk, "ABCDEFGHIJKLMNOP", 16);

    char buffer[512];
    char cbuffer[32];

    if (argc == 2) {
      add_ip(&liste, argv[1], NULL);
    } else {
      border_getNodes(&liste);
    }
    border_printNodes(liste);

    char run = 1;
    while (run) {
        if (fgets(cbuffer,32,stdin) == NULL) continue;
        int length = strlen(cbuffer) - 1;
        char unknown = 1;
        switch (length) {
            case 2:
                if (!memcmp("ls", cbuffer, 2)) {
                    border_getNodes(&liste);
                    border_printNodes(liste);
                    unknown = 0;
                }
                break;
            case 4:
                if (!memcmp("exit", cbuffer, 4)) {
                    run = 0;
                    unknown = 0;
                }
                break;
            case 5:
                if (!memcmp("psk", cbuffer, 3)) {
                    uint8_t *ip = get_ip(liste, atoi(cbuffer + 4));
                    memset(buffer, 0, 512);
                    node_getPSK(ip, buffer);
                    printf("PSK: ");
                    int i;
                    for (i = 0; i < 16; i++) printf("%02X", buffer[i]);
                    printf("\n");
                    unknown = 0;
                }
                break;
            case 6:
                if (!memcmp("core", cbuffer, 4)) {
                    uint8_t *ip = get_ip(liste, atoi(cbuffer + 5));
                    memset(buffer, 0, 512);
                    node_getCore(ip, buffer);
                    int i;
                    for (i = 0; i < 512; i++) if (buffer[i] == ',') buffer[i] = '\n';
                    printf("Core:\n%s\n", buffer);
                    unknown = 0;
                }
                if (!memcmp("name", cbuffer, 4)) {
                    uint8_t *ip = get_ip(liste, atoi(cbuffer + 5));
                    memset(buffer, 0, 512);
                    node_getName(ip, buffer);
                    printf("Name: %s\n", buffer);
                    unknown = 0;
                }
                if (!memcmp("uuid", cbuffer, 4)) {
                    uint8_t *ip = get_ip(liste, atoi(cbuffer + 5));
                    memset(buffer, 0, 512);
                    node_getUUID(ip, buffer);
                    char uuid[37];
                    uuid_unparse((unsigned char *) buffer, uuid);
                    printf("UUID: %s\n", uuid);
                    unknown = 0;
                }
                if (!memcmp("time", cbuffer, 4)) {
                    uint8_t *ip = get_ip(liste, atoi(cbuffer + 5));
                    memset(buffer, 0, 512);
                    node_getTime(ip, buffer);
                    struct tm *timeinfo = localtime((const time_t *) buffer);
                    char b[64];
                    memset(b, 0, 64);
                    strftime(b, 64, "%d.%m.%Y, %H:%M:%S", timeinfo);
                    printf("%s\n", b);
                    unknown = 0;
                }
                break;
            case 7:
                if (!memcmp("model", cbuffer, 5)) {
                    uint8_t *ip = get_ip(liste, atoi(cbuffer + 6));
                    memset(buffer, 0, 512);
                    node_getModel(ip, buffer);
                    printf("Model: %s\n", buffer);
                    unknown = 0;
                }
                if (!memcmp("flash", cbuffer, 5)) {
                    uint8_t *ip = get_ip(liste, atoi(cbuffer + 6));
                    node_firmware(ip, "server_redbee-econotag.bin");
                    printf("Flash erfolgreich. Gerät wird neu gestartet ");
                    fflush(stdout);
                    sleep(1);
                    int i;
                    for (i = 0; i < 11; i++) {
                        printf(".");
                        fflush(stdout);
                        sleep(1);
                    }
                    printf(" Abgeschlossen\n");
                    unknown = 0;
                }
                break;
            case 9:
                if (!memcmp("settime", cbuffer, 7)) {
                    uint8_t *ip = get_ip(liste, atoi(cbuffer + 8));
                    memset(buffer, 0, 512);
                    node_setTime(ip, buffer);
                    printf("Zeitupdate gesendet\n");
                    unknown = 0;
                }
                break;
            case 11:
                if (!memcmp("handshake", cbuffer, 9)) {
                    uint8_t *ip = get_ip(liste, atoi(cbuffer + 10));
                    node_handshake(ip);
                    unknown = 0;
                }
                break;
        }
        if (unknown) printf("Unbekannter Befehl. Möglichkeiten:\n   ls\n   handshake <nr>\n   name <nr>\n   ecc <nr>\n   uuid <nr>\n   time <nr>\n   model <nr>\n   flash <nr>\n   exit\n");
    }    

    clear_ip(&liste);
    return 0;
}
