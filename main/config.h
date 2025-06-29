/**
 * @file config.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-06-28
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef __CONFIG_H_
#define __CONFIG_H_

#include "driver/gpio.h"

#define I2C_SCL_GPIO    GPIO_NUM_3
#define I2C_SDA_GPIO    GPIO_NUM_4
#define BEEP_GPIO       GPIO_NUM_1

#define DEFAULT_I2C_CLOCK         (100000)


#endif
