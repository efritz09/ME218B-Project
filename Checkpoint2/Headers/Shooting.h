/****************************************************************************
 Header file for shooting state machine

 ****************************************************************************/

#ifndef Shooting_H
#define Shooting_H

// State definitions for use with the query function
typedef enum { FindBeaconState, FoundBeaconRightState, FoundBeaconLeftState, LoadingBallState, FireState } ShootingState ;

// Public Function Prototypes

ES_Event RunShooting( ES_Event CurrentEvent );
void StartShooting ( ES_Event CurrentEvent );
ShootingState QueryShooting ( void );

#endif /*Shooting_H */

