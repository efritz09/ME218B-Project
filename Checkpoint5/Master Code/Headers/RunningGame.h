/****************************************************************************
 Header file for running game state machine

 ****************************************************************************/

#ifndef RunningGame_H
#define RunningGame_H


// State definitions for use with the query function
typedef enum { DrivingStateSM, ShootingStateSM, ObstacleStateSM } RunningGameState;

// Public Function Prototypes

ES_Event RunRunningGame( ES_Event CurrentEvent );
void StartRunningGame ( ES_Event CurrentEvent );
RunningGameState QueryRunningGame( void );

#endif /*RunningGame_H */

