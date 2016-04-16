//BallShooter.h

/****************************************************************************
  
  Header file for PWM.c, which controlls all of our motors
 
 ****************************************************************************/
#ifndef BallShooter_H
#define BallShooter_H


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

/*----------------------------- Module Defines ----------------------------*/
// Any enum or struct required for module
typedef enum {
	ShooterReady,
	ShooterResetting,
}ShooterState;


/*----------------------- Public Function Prototypes ----------------------*/


bool InitBallShooter ( void );
ES_Event RunBallShooter ( ES_Event ThisEvent );				
bool PostBallShooter ( ES_Event ThisEvent );					

void FireBallShooter(void);
#endif
