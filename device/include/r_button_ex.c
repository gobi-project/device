#include "lib/sensors.h"
#include "mc1322x.h"
#include <signal.h>

/* SENSOR ------------------------------------------------------------------ */

const struct sensors_sensor externbutton_sensor;

static struct timer externbutton_debouncetimer;
static int status_externbutton(int type);

void kbi6_isr(void)  {
	if(timer_expired(&externbutton_debouncetimer)) {
		timer_set(&externbutton_debouncetimer, CLOCK_SECOND / 4);
		sensors_changed(&externbutton_sensor);
	}
	clear_kbi_evnt(6);
}

static int value_externbutton(int type) {
	return GPIO->DATA.GPIO_28 || !timer_expired(&externbutton_debouncetimer);
}

static int configure_externbutton(int type, int c) {
	switch (type) {
	case SENSORS_HW_INIT:
		if (c) {
			disable_irq_kbi(6);
		} else {
			if(!status_externbutton(SENSORS_ACTIVE)) {
				timer_set(&externbutton_debouncetimer, 0);
				enable_irq_kbi(6);
				kbi_edge(6);
				enable_ext_wu(6);
			}
		}
		return 1;
	}
	return 0;
}

static int status_externbutton(int type) {
	switch (type) {
		case SENSORS_ACTIVE:
		case SENSORS_READY:
			return bit_is_set(*CRM_WU_CNTL, 22); /* check if kbi6 irq is enabled */
	}
	return 0;
}

SENSORS_SENSOR(externbutton_sensor, "ExternButton", value_externbutton, configure_externbutton, status_externbutton);

/* RESOURCE ---------------------------------------------------------------- */

void button_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  uint8_t result = 1; 

  int length = 0;
  
  uint8_t source_string[LEN_SENML_BUTTON]; // {"bn":"/btn","bu":"B","e":[{"v":"%d"}]}
  flash_getVar(source_string, RES_SENML_BUTTON, LEN_SENML_BUTTON);
  length = snprintf(buffer, REST_MAX_CHUNK_SIZE, source_string, result);

  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_response_payload(response, buffer, length);
}
void button_event_handler();

EVENT_RESOURCE(res_btn, "rt=\"gobi.s.swt\";if=\"core.s\";obs", button_resource_handler, 0, 0, 0, button_event_handler);

void button_event_handler() {
  REST.notify_subscribers(&res_btn);
}
