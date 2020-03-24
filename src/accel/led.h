#ifndef LED_H
#define LED_H

#include <stdbool.h>

typedef enum {
    GRN_LED = 0,
    BLU_LED = 1,
    RED_LED = 2
} Led;

void init_led(void);
void set_led(Led led, bool on);

#endif
