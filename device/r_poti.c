#include "lib/sensors.h"
#include "mc1322x.h"
#include <signal.h>
#include <adc.h>

#define ADC_PIN 0

/* SENSOR ------------------------------------------------------------------ */

const struct sensors_sensor poti_sensor;

static int poti_value(int type) 
{
	uint16_t result = 0;
	uint8_t counter = 0;

    global_irq_disable();

	while(counter < 254)
	{
      adc_service();
	  result = adc_voltage(ADC_PIN);
	  counter++;
	  clock_delay_usec( 33 );
	}

    global_irq_enable();

	return result;
}

static int poti_configure(int type, int c) 
{
	switch (type) {
	case SENSORS_HW_INIT:
		if (c) 
		{
			adc_disable();
		} 
		else 
		{
			adc_enable();
			adc_init();
			adc_setup_chan( ADC_PIN );
		}
		return 1;
	}
	return 0;
}

static int poti_status(int type) 
{
	switch (type) 
	{
		case SENSORS_ACTIVE:
		case SENSORS_READY:
			return 1;
	}
	return 0;
}

SENSORS_SENSOR(poti, "Potentiometer", poti_value, poti_configure, poti_status);

/* RESOURCE ---------------------------------------------------------------- */

void poti_resource_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  int length = 0;

  int result = (poti.value(0) * 100) / 35;

  uint8_t source_string[LEN_SENML_VAL]; // {"bn":"/val","bu":"%%","e":[{"v":"%d.%d"}]}
  nvm_getVar(source_string, RES_SENML_VAL, LEN_SENML_VAL);
  length = snprintf(buffer, REST_MAX_CHUNK_SIZE, source_string, result / 100, result % 100 );

  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_response_payload(response, buffer, length);
}
void  poti_periodic_handler();

PERIODIC_RESOURCE(res_poti, "rt=\"gobi.s.val\";if=\"core.s\";obs", poti_resource_handler, NULL, NULL, NULL, CLOCK_SECOND, poti_periodic_handler);

void poti_periodic_handler() {
  REST.notify_subscribers(&res_poti);
}
