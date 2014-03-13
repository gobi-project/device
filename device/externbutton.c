#include "lib/sensors.h"
#include "mc1322x.h"
#include <signal.h>

const struct sensors_sensor externbutton_sensor;

static struct timer externbutton_debouncetimer;
static int status_externbutton(int type);

void kbi6_isr(void) 
{
	if(timer_expired(&externbutton_debouncetimer)) 
	{
		timer_set(&externbutton_debouncetimer, CLOCK_SECOND / 4);
		sensors_changed(&externbutton_sensor);
	}
	clear_kbi_evnt(6);
}

static int
value_externbutton(int type)
{
	return GPIO->DATA.GPIO_28 || !timer_expired(&externbutton_debouncetimer);
}

static int
configure_externbutton(int type, int c)
{
	switch (type) {
	case SENSORS_HW_INIT:
		if(c == 0) {
			if(!status_externbutton(SENSORS_ACTIVE)) {
				timer_set(&externbutton_debouncetimer, 0);
				enable_irq_kbi(6);
				kbi_edge(6);
				enable_ext_wu(6);
			}
		} else {
			disable_irq_kbi(6);
		}
		return 1;
	}
	return 0;
}

static int
status_externbutton(int type)
{
	switch (type) 
	{
		case SENSORS_ACTIVE:
		case SENSORS_READY:
			return bit_is_set(*CRM_WU_CNTL, 22); /* check if kbi6 irq is enabled */
	}
	return 0;
}

SENSORS_SENSOR(externbutton_sensor, "ExternButton", value_externbutton, configure_externbutton, status_externbutton);