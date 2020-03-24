#include <devices.h>
#include "led.h"

// These items are part of "project_cfg.h", so they must be the same
// here.

// For RGB LED.
#define BLU_GPIONUM	(4)
#define GRN_GPIONUM	(5)
#define RED_GPIONUM	(6) 

extern handle_t gio;

// Notice that this relies upon the "gio" from "project_cfg.h", so we
// must be sure to initialize the LCD before we call this.

void init_led(void) {

    configASSERT(gio);
    gpio_set_drive_mode(gio, GRN_GPIONUM, GPIO_DM_OUTPUT);
    gpio_set_pin_value(gio, GRN_GPIONUM, GPIO_PV_HIGH);
    gpio_set_drive_mode(gio, BLU_GPIONUM, GPIO_DM_OUTPUT);
    gpio_set_pin_value(gio, BLU_GPIONUM, GPIO_PV_HIGH);
    gpio_set_drive_mode(gio, RED_GPIONUM, GPIO_DM_OUTPUT);
    gpio_set_pin_value(gio, RED_GPIONUM, GPIO_PV_HIGH);
}

void set_led(Led led, bool on) {
    switch(led) {
    case 0:
	gpio_set_pin_value(gio, GRN_GPIONUM, on? GPIO_PV_LOW : GPIO_PV_HIGH);
	break;
    case 1:
	gpio_set_pin_value(gio, BLU_GPIONUM, on? GPIO_PV_LOW : GPIO_PV_HIGH);
	break;
    case 2:
	gpio_set_pin_value(gio, RED_GPIONUM, on? GPIO_PV_LOW : GPIO_PV_HIGH);
    }
}
	    
