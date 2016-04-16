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
#define HALF_SPEED_PORT 71
#define QUARTER_SPEED_PORT 50
#define FULL_SPEED_PORT 100
#define HALF_SPEED_STARBOARD 75
#define QUARTER_SPEED_STARBOARD 53
#define FULL_SPEED_STARBOARD 100

#define ONE_SEC 976
#define ROTATION_TIME (ONE_SEC*2-190) //ONE_SEC*2 - 190
#define BANK_90_DEGREE_TIME (1800)
#define PIXELS_PER_3SEC (115)

void InitPWM(void);
void SetPWMDuty(uint8_t duty, int channel);
void SetPWMWidth(uint32_t width, int channel);

void SetLastPWM(uint8_t lastPWM, int channel);
uint8_t GetLastPWM(int channel);

void SetLastDirection(int dir, int channel);
int GetLastDirection(int channel);

#endif
