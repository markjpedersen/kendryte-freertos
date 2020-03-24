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
#ifndef _PROJECT_CFG_H_
#define _PROJECT_CFG_H_
#include <pin_cfg.h>

#define SPI_CHANNEL 0
#define SPI_SLAVE_SELECT 3
#define __SPI_SYSCTL(x, y) SYSCTL_##x##_SPI##y
#define _SPI_SYSCTL(x, y) __SPI_SYSCTL(x, y)
#define SPI_SYSCTL(x) _SPI_SYSCTL(x, SPI_CHANNEL)
#define __SPI_SS(x, y) FUNC_SPI##x##_SS##y
#define _SPI_SS(x, y) __SPI_SS(x, y)
#define SPI_SS _SPI_SS(SPI_CHANNEL, SPI_SLAVE_SELECT)
#define __SPI(x, y) FUNC_SPI##x##_##y
#define _SPI(x, y) __SPI(x, y)
#define SPI(x) _SPI(SPI_CHANNEL, x)

#define DCX_IO          (38)
#define DCX_GPIONUM     (2)

// For DAN board.
#define RST_IO		(37)
#define RST_GPIONUM	(3)

// For RGB LED.
#define BLU_IO		(12)
#define GRN_IO		(13)
#define RED_IO		(14)
#define BLU_GPIONUM	(4)
#define GRN_GPIONUM	(5)
#define RED_GPIONUM	(6) 

const power_bank_cfg_t g_power_bank_cfg =
{
    .version = PIN_CFG_VERSION,
    .power_banks_count = 2,
    .power_banks =
    {
	{SYSCTL_POWER_BANK6, SYSCTL_POWER_V18},
	{SYSCTL_POWER_BANK7, SYSCTL_POWER_V18}
    }
};
const fpioa_cfg_t g_fpioa_cfg =
{
    .version = PIN_CFG_VERSION,
    .functions_count = 9,
    .functions =
    {
        {DCX_IO, FUNC_GPIOHS0 + DCX_GPIONUM},	// for LCD
        {36, SPI_SS},
        {39, SPI(SCLK)},
	{RST_IO, FUNC_GPIOHS0 + RST_GPIONUM},

	{9, FUNC_I2C0_SCLK},			// for IMU
	{10, FUNC_I2C0_SDA},

	{BLU_IO, FUNC_GPIOHS0 + BLU_GPIONUM},	// for blue LED
	{GRN_IO, FUNC_GPIOHS0 + GRN_GPIONUM},	// for green LED
	{RED_IO, FUNC_GPIOHS0 + RED_GPIONUM}	// for red LED
    }
};
const pin_cfg_t g_pin_cfg =
{
    .version = PIN_CFG_VERSION,
    .set_spi0_dvp_data = 1
};

#endif
