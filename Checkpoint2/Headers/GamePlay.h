/****************************************************************************
 Header file for gameplay state machine

 ****************************************************************************/

#ifndef GamePlay_H
#define GamePlay_H


// State definitions for use with the query function
typedef enum { DrivingStateGP, ShootingStateGP, ObstacleStateGP, PauseStateGP } GamePlayState ;

// Public Function Prototypes

ES_Event RunGamePlay( ES_Event CurrentEvent );
void StartGamePlay ( ES_Event CurrentEvent );
GamePlayState QueryGamePlay( void );

#endif /*GamePlay_H */

