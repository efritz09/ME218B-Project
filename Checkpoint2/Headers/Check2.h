/****************************************************************************
  
  Header file for Check2.c, which controlls all of our motors
 
 ****************************************************************************/
#ifndef C2_H
#define C2_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// ES_framework headers
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Port.h"
#include "ES_Types.h"
#include "ES_Events.h"

// TIVA headers
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "inc/hw_nvic.h"
#include "inc/hw_timer.h"
#include "bitdefs.h"
#include "driverlib/gpio.h" 
/*----------------------------- Module Defines ----------------------------*/
// Any enum or struct required for module
typedef enum {IdleState, StraightState, TurnState} C2State;

/*----------------------- Public Function Prototypes ----------------------*/

bool InitCheck2 ( uint8_t Priority );
ES_Event RunCheck2( ES_Event ThisEvent );
bool PostCheck2 ( ES_Event ThisEvent );

#endif
