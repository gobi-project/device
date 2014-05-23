#include <string.h>

#include <rest-engine.h>
#include <er-coap.h>
#include <er-coap-separate.h>
#include <er-coap-transactions.h>

#include "storage.h"
#include "mc1322x.h"
#include "clock.h"
#include "ecc.h"
#include "er-dtls-psk.h"
#include "flash-store.h"

void device_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
    const char *uri_path = NULL;
    uint8_t uri_len = REST.get_url(request, &uri_path);

    if (uri_len == 1) {
        if (*offset >= LEN_D_CORE) {
            coap_set_status_code(response, BAD_OPTION_4_02);
            coap_set_payload(response, "BlockOutOfScope", 15);
            return;
        }

        nvm_getVar(buffer, RES_D_CORE + *offset, preferred_size);
        if (LEN_D_CORE - *offset < preferred_size) {
            preferred_size = LEN_D_CORE - *offset;
            *offset = -1;
        } else {
            *offset += preferred_size;
        }

        REST.set_header_content_type(response, APPLICATION_LINK_FORMAT);
        REST.set_response_payload(response, buffer, preferred_size);
        return;
    }

    //*************************************************************************
    //*  DEVICE NAME                                                          *
    //*************************************************************************
    if (uri_path[2] == 'n' && uri_path[3] == 'a') {
        nvm_getVar(buffer, RES_NAME, LEN_NAME);
        REST.set_header_content_type(response, TEXT_PLAIN);
        REST.set_response_payload(response, buffer, strlen(buffer));
    }

    //*************************************************************************
    //*  DEVICE MODEL                                                         *
    //*************************************************************************
    if (uri_path[2] == 'm') {
        nvm_getVar(buffer, RES_MODEL, LEN_MODEL);
        REST.set_header_content_type(response, TEXT_PLAIN);
        REST.set_response_payload(response, buffer, strlen(buffer));
        return;
    }

    //*************************************************************************
    //*  DEVICE IDENTIFIER                                                    *
    //*************************************************************************
    if (uri_path[2] == 'u') {
        leds_on(LEDS_GREEN);

        nvm_getVar(buffer, RES_UUID, LEN_UUID);
        REST.set_header_content_type(response, APPLICATION_OCTET_STREAM);
        REST.set_response_payload(response, buffer, LEN_UUID);
    }

    //*************************************************************************
    //*  DEVICE TIME                                                          *
    //*************************************************************************
    if (uri_path[2] == 't') {
        uint32_t time = uip_htonl(clock_seconds());
        memcpy(buffer, &time, 4);
        REST.set_header_content_type(response, APPLICATION_OCTET_STREAM);
        REST.set_response_payload(response, buffer, 4);
    }

    //*************************************************************************
    //*  DEVICE PSK                                                           *
    //*************************************************************************
    if (uri_path[2] == 'p') {
        getPSK(buffer);
        REST.set_header_content_type(response, APPLICATION_OCTET_STREAM);
        REST.set_response_payload(response, buffer, LEN_PSK);
    }

    //*************************************************************************
    //*  DEVICE DEFAULT ROUTE                                                 *
    //*************************************************************************
    if (uri_path[2] == 'r') {
        uip_ipaddr_t *addr = uip_ds6_defrt_choose();
        memcpy(buffer, addr, 16);
        REST.set_header_content_type(response, APPLICATION_OCTET_STREAM);
        REST.set_response_payload(response, buffer, 16);
    }

    //*************************************************************************
    //*  DEVICE NEIGHBOURS                                                    *
    //*************************************************************************
    if (uri_path[2] == 'n' && uri_path[3] == 'b') {
        uint32_t o = *offset / 16;
        uint32_t bfr_ptr = 0;
        uint32_t i;
        uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors);
        for (i = 0; i < (o+2); i++) {
            if (i >= o) {
                memcpy(buffer + bfr_ptr, &nbr->ipaddr, 16);
                bfr_ptr += 16;
            }

            nbr = nbr_table_next(ds6_neighbors, nbr);
            if (nbr == NULL) {
                if (bfr_ptr == 0) {
                  coap_set_status_code(response, BAD_OPTION_4_02);
                  coap_set_payload(response, "BlockOutOfScope", 15);
                  return;
                }
                break;
            }
        }
        if (nbr == NULL) {
            *offset = -1;
        } else {
           *offset += bfr_ptr;
        }
        REST.set_header_content_type(response, APPLICATION_OCTET_STREAM);
        REST.set_response_payload(response, buffer, bfr_ptr);
    }
}

PARENT_RESOURCE(res_device, "rt=\"dev.info\";if=\"core.ll\"", device_handler, NULL, NULL, NULL);

void time_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
    const uint8_t *payload = 0;
    size_t pay_len = REST.get_request_payload(request, &payload);

    if (payload && pay_len == 4) {
        uint32_t time;
        memcpy(&time, payload, 4);
        time = uip_ntohl(time);
        clock_set_seconds(time);
        REST.set_response_status(response, CHANGED_2_04);
    } else {
        REST.set_response_status(response, BAD_REQUEST_4_00);
        memcpy(buffer, "32 bit Unixzeit in Network-Byte-Order benötigt.", 48);
        REST.set_response_payload(response, buffer, 48);
    }
}

RESOURCE(res_time, "rt=\"dev.time.update\";if=\"core.u\"", NULL, time_handler, NULL, NULL);
