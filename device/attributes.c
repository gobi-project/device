#include <string.h>

#include <rest-engine.h>
#include <er-coap.h>
#include <er-coap-separate.h>
#include <er-coap-transactions.h>

#include "mc1322x.h"
#include "clock.h"
#include "flash-store.h"
#include "ecc.h"
#include "er-dtls-psk.h"

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
    if (uri_path[2] == 'n') {
        nvm_getVar(buffer, RES_NAME, LEN_NAME);
        buffer[REST_MAX_CHUNK_SIZE - 1] = 0;
        REST.set_header_content_type(response, TEXT_PLAIN);
        REST.set_response_payload(response, buffer, LEN_NAME);
    }

    //*************************************************************************
    //*  DEVICE MODEL                                                         *
    //*************************************************************************
    if (uri_path[2] == 'm') {
        nvm_getVar(buffer, RES_MODEL, LEN_MODEL);
        buffer[REST_MAX_CHUNK_SIZE - 1] = 0;
        REST.set_header_content_type(response, TEXT_PLAIN);
        REST.set_response_payload(response, buffer, LEN_MODEL);
        return;
    }

    //*************************************************************************
    //*  DEVICE IDENTIFIER                                                    *
    //*************************************************************************
    if (uri_path[2] == 'u') {
        nvm_getVar(buffer, RES_UUID, LEN_UUID);
        buffer[REST_MAX_CHUNK_SIZE - 1] = 0;
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
        buffer[REST_MAX_CHUNK_SIZE - 1] = 0;
        REST.set_header_content_type(response, APPLICATION_OCTET_STREAM);
        REST.set_response_payload(response, buffer, LEN_PSK);
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
