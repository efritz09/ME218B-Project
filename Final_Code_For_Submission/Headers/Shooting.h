/****************************************************************************
 Header file for shooting state machine

 ****************************************************************************/

#ifndef Shooting_H
#define Shooting_H

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Types.h"
#include "ES_Port.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "inc/hw_timer.h"
#include "inc/hw_nvic.h"
#include "bitdefs.h"
#include "termio.h"

// State definitions for use with the query function
typedef enum { OrientState, FindYState, ShootingTurningState, FindXState,
								FindBeaconState, FireState, ReturnToTapeState } ShootingState ;

// Public Function Prototypes
ES_Event RunShooting( ES_Event CurrentEvent );
void StartShooting ( ES_Event CurrentEvent );
ShootingState QueryShooting ( void );
bool InitBeaconCaptureResponse (void);

#endif /*Shooting_H */

