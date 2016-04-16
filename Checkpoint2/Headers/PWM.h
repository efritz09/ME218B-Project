/****************************************************************************
  
  Header file for PWM.c, which controlls all of our motors
 
 ****************************************************************************/
#ifndef PWM_H
#define PWM_H

/*----------------------------- Module Defines ----------------------------*/
// Any enum or struct required for module

/*----------------------- Public Function Prototypes ----------------------*/

// Symbolic defines for DC motor speeds (in duty cycle)
#define PORT_MOTOR 				1
#define STARBOARD_MOTOR 		0
#define HALF_SPEED_PORT 		75
#define FULL_SPEED_PORT 		100
#define HALF_SPEED_STARBOARD 	75
#define FULL_SPEED_STARBOARD 	100
#define ONE_SEC 987

#include <stdint.h>

void InitPWM(void);
void SetPWMDuty(uint8_t duty, int channel);


#endif

