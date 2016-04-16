/****************************************************************************
 Header file for driving state machine

 ****************************************************************************/

#ifndef Driving_H
#define Driving_H

// State definitions for use with the query function
typedef enum { AtPositionState, MovingState } DrivingState ;
typedef enum {BottomStraight, RightStraight, TopStraight, LeftStraight, ShootingZone, ObstacleZone} MapSection;

// Define point structure
typedef struct {
			uint16_t 		X;
			uint16_t 		Y;
} POINT_t;

// Public Function Prototypes

MapSection FindSection(POINT_t current);

ES_Event RunDriving( ES_Event CurrentEvent );
void StartDriving ( ES_Event CurrentEvent );
DrivingState QueryDriving ( void );

#endif /*Driving_H */

