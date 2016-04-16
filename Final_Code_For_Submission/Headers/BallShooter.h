/****************************************************************************
  
  Header file for BallShooter.c, which controlls the servo motors
 
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

/*----------------------------- Module Defines ----------------------------*/
typedef enum {ShooterReady, ShooterResetting}ShooterState;

/*----------------------- Public Function Prototypes ----------------------*/
bool InitBallShooter ( void );
ES_Event RunBallShooter ( ES_Event ThisEvent );									
void FireBallShooter(void);

#endif /*BallShooter_H*/
