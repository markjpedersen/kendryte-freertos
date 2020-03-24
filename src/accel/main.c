/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <devices.h>
#include "lcd.h"
#include "led.h"

handle_t i2c;
float a[3];

void poll_accelerometer(void *pvParameters) {

    // Get the LSM303 accelerometer at adrs 0x19, and set
    // high-resolution mode.

    uint8_t reg;
    uint8_t buf[3];
    const TickType_t delay = 10/portTICK_PERIOD_MS;
    TickType_t xLastWake;
    
    handle_t accel = i2c_get_device(i2c, 0x19, 7);
    i2c_dev_set_clock_rate(accel, 50000);
    reg = 0x23;				// CTRL_REG4_A
    buf[0] = reg;
    buf[1] = 0x08;			// set HR (bit 3)
    io_write(accel, buf, 2);

    // Set the LSM303 data rate to 1 sec. (reg. 0x20, val. 0x17).
    
    reg = 0x20;				// CTRL_REG1_A
    buf[0] = reg;
    buf[1] = 0x17;			// set ODR0, Zen, Yen, Xen.
    io_write(accel, buf, 2);
    
    xLastWake = xTaskGetTickCount();

    while (1) {

	set_led(GRN_LED, true);

	// Check the accelerometer ready.

	reg = 0x27;
	i2c_dev_transfer_sequential(accel, &reg, 1, buf, 1);
	if (buf[0] & 0x08) {

	    // Read X, Y, and Z accels.  Default FS is +/- 2 G, or 1
	    // mg/LSB.  Use the multiple-byte, auto-increment register
	    // mode by setting the MSB of the register address.  This way
	    // we can read all six sequential registers at a time.

	    int16_t araw[3];
	    
	    reg = 0x28 | 0x80;
	    i2c_dev_transfer_sequential(accel, &reg, 1, (uint8_t*)araw, 6);
	    a[0] = araw[0] >> 4;
	    a[1] = araw[1] >> 4;
	    a[2] = araw[2] >> 4;
	}

	set_led(GRN_LED, false);
	
	// Wait 10 ms.

	vTaskDelayUntil(&xLastWake, delay);
    }
}

void update_output(void *pvParameters) {

    const TickType_t delay = 1000/portTICK_PERIOD_MS;
    TickType_t xLastWake;
    int i = 0;
    char astr[80] = "";

    xLastWake = xTaskGetTickCount();

    while (1) {

	set_led(RED_LED, true);

	// Update the LCD.

	lcd_clear(BLACK);
	lcd_draw_string(100, 50, "Accel test",  RED);
	sprintf(astr, "(%f, %f, %f)", a[0], a[1], a[2]);
	lcd_draw_string(5, 90, astr, WHITE);
	lcd_draw_rectangle(150, 150, 170, 170, 1, WHITE);

	// Update the serial port.

	printf("%d:  %s\n\r", i++, astr);
	set_led(RED_LED, false);

	// Wait 1 s.
	
	vTaskDelayUntil(&xLastWake, delay);
    }
}
	

int main(void)
{
    const TickType_t delay = 1000/portTICK_PERIOD_MS;

    printf("Accel test\n\r");
    lcd_init();
    init_led();
    /* memset(lcd_gram, 0xc0, LCD_X_MAX * LCD_Y_MAX * 2); */
    /* lcd_draw_picture(100, 0, 120, 240, lcd_gram); */

    
    // Try to read the L3GD20H "WHO_AM_I" register (adrs 0x6b, reg
    // 0x0f; val 0xd7).

    uint8_t reg = 0x0f;
    uint8_t buf[3] = "";
    i2c = io_open("/dev/i2c0");
    handle_t gyro = i2c_get_device(i2c, 0x6b, 7);

    i2c_dev_set_clock_rate(gyro, 50000);
    i2c_dev_transfer_sequential(gyro, &reg, 1, buf, 1);
    configASSERT(buf[0] == 0xd7);
//    printf("WHO_AM_I:  %02x\n\r", buf[0]);

    // Create the two tasks for polling the accelerometer and updating
    // the output.  No need to start the scheduler, since it's already
    // running.

    xTaskCreate(poll_accelerometer, "Poll accel", 1000, NULL, 2, NULL);
    xTaskCreate(update_output, "Update out", 1000, NULL, 1, NULL);

    for (int i=0, il=0; true; il=i, i=(i+1)%3) {
	
	/* set_led(il, false); */
	/* set_led(i, true); */
	vTaskDelay(delay);
    }
}
