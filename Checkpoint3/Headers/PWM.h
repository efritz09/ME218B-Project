/****************************************************************************
  Header file for PWM.c
 ****************************************************************************/
#ifndef PWM_H
#define PWM_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_pwm.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "bitdefs.h"

#define PORT_MOTOR 1
#define STARBOARD_MOTOR 0
#define HALF_SPEED_PORT 75
#define FULL_SPEED_PORT 100
#define HALF_SPEED_STARBOARD 70
#define FULL_SPEED_STARBOARD 100

#define ONE_SEC 976
#define ROTATION_TIME (ONE_SEC*2-175)
#define PIXELS_PER_3SEC (130)

#define PosResolution 10
#define AngleResolution 10

void InitPWM(void);
void SetPWMDuty(uint8_t duty, int channel);
void setPWMWidth(uint32_t width, int channel);

#endif
