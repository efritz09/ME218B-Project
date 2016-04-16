/****************************************************************************
 
  Wrapper header file for all headers in the master code project

 ****************************************************************************/

#ifndef H_H
#define H_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <cmath>

// Framework Headers
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Port.h"
#include "ES_Types.h"
#include "ES_Events.h"

// TIVA Headers
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_pwm.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "inc/hw_ssi.h"
#include "inc/hw_nvic.h"
#include "driverlib/ssi.h"
#include "bitdefs.h"

// State Machines
#include "Master.h"
#include "GamePlay.h"
#include "SPITemplate.h"
#include "RunningGame.h"
#include "Driving.h"
#include "Shooting.h"
#include "Obstacle.h"
#include "BallShooter.h"

// Services
#include "Drive.h"
#include "PWM.h"
#include "Points.h"
#include "DriveAlgorithm.h"
#include "ADMulti.h"

// Defines
#define ONE_SEC 976
#define ALL_BITS (0xff<<2)

#define BitsPerNibble  4
#define TicksPerMS 40000

#endif /* H_H */
