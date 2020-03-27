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
#include <semphr.h>
#include <devices.h>
#include "lcd.h"
#include "led.h"

handle_t i2c;
float a[3];
SemaphoreHandle_t acc_int_semaphore = NULL;

void sample_accelerometer(void *pvParameters) {

    // Get the LSM303 accelerometer at adrs 0x19, and set
    // high-resolution mode.

    uint8_t reg;
    uint8_t buf[3];
    
    handle_t accel = i2c_get_device(i2c, 0x19, 7);
    i2c_dev_set_clock_rate(accel, 400000);
    
    reg = 0x23;				// CTRL_REG4_A
    buf[0] = reg;
    buf[1] = 0x08;			// set HR (bit 3)
    io_write(accel, buf, 2);

    // Set the LSM303 data rate to 50 Hz. (reg. 0x20, val. 0x47).
    
    reg = 0x20;				// CTRL_REG1_A
    buf[0] = reg;
    buf[1] = 0x47;			// set ODR0, Zen, Yen, Xen.
    io_write(accel, buf, 2);

    // Set the LSM303 INT1 pin to report DRDY1, whatever that is.
    // This is reg 0x22, val. 0x10.

    reg = 0x22;				// CTRL_REG3_A
    buf[0] = reg;
    buf[1] = 0x10;			// set I1_DRDY1
    io_write(accel, buf, 2);

    // Create semaphore for signaling with ISR.

    acc_int_semaphore = xSemaphoreCreateBinary();
    
    while (1) {

	// Block on semaphore from ISR.

	if (xSemaphoreTake(acc_int_semaphore, portMAX_DELAY) == pdTRUE) {
	    
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

	    set_led(GRN_LED, false);
	}
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
	
// Callback for accelerometer interrupt.

// For IMU interrupts.
#define ACC_INT_IO	(15)
#define ACC_INT_GPIONUM	(7)

extern handle_t gio;

void handle_acc_int(uint32_t pin, void* userdata) {

    static BaseType_t deferredTaskWoken;
    set_led(GRN_LED, true);
    xSemaphoreGiveFromISR(acc_int_semaphore, &deferredTaskWoken);
    portYIELD_FROM_ISR();
}
 

int main(void)
{
    const TickType_t delay = 1000/portTICK_PERIOD_MS;

    printf("Accel test\n\r");
    lcd_init();
    init_led();

    // Set up Accelerometer interrupt.  We need to do this after
    // lcd_init(), since that initializes the GPIO driver.

    gpio_set_drive_mode(gio, ACC_INT_GPIONUM, GPIO_DM_INPUT);
    gpio_set_pin_edge(gio, ACC_INT_GPIONUM, GPIO_PE_RISING);
    gpio_set_on_changed(gio, ACC_INT_GPIONUM, handle_acc_int, NULL);
    
    uint8_t reg = 0x0f;
    uint8_t buf[3] = "";
    i2c = io_open("/dev/i2c0");
    handle_t gyro = i2c_get_device(i2c, 0x6b, 7);

    i2c_dev_set_clock_rate(gyro, 50000);
    i2c_dev_transfer_sequential(gyro, &reg, 1, buf, 1);
    configASSERT(buf[0] == 0xd7);

    // Create the two tasks for polling the accelerometer and updating
    // the output.  No need to start the scheduler, since it's already
    // running.

    xTaskCreate(sample_accelerometer, "Sample accel", 1000, NULL, 10, NULL);
    xTaskCreate(update_output, "Update out", 1000, NULL, 1, NULL);

    while(1) {
	vTaskDelay(delay);
    }
 }
