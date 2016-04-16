/****************************************************************************
 Header file for driving state machine

 ****************************************************************************/

#ifndef Driving_H
#define Driving_H

typedef enum { AtPositionState, GeneratePathState, TurningState, DrivingForwardState } DrivingState ;

// Public Function Prototypes
ES_Event RunDriving( ES_Event CurrentEvent );
void StartDriving ( ES_Event CurrentEvent );
DrivingState QueryDriving ( void );

#endif /*Driving_H */

