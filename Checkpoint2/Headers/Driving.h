/****************************************************************************
 Header file for driving state machine

 ****************************************************************************/

#ifndef Driving_H
#define Driving_H

// State definitions for use with the query function
typedef enum { AtPositionState, MovingState } DrivingState ;

// Define point structure
typedef struct {
			uint16_t 		X;
			uint16_t 		Y;
} POINT_t;

// Public Function Prototypes

ES_Event RunDriving( ES_Event CurrentEvent );
void StartDriving ( ES_Event CurrentEvent );
DrivingState QueryDriving ( void );

#endif /*Driving_H */

