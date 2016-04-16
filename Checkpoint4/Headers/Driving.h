/****************************************************************************
 Header file for driving state machine

 ****************************************************************************/

#ifndef Driving_H
#define Driving_H

// State definitions for use with the query function
typedef enum { AtPositionState, MovingState } DrivingState ;
typedef enum {BottomStraight, RightStraight, TopStraight, LeftStraight, ShootingZone, ObstacleZone} MapSection;
typedef enum {NoDecision, ShootingDecisionZone, ObstacleDecisionZone} MapDecisionZone;

// Define point structure
typedef struct {
			uint16_t 		X;
			uint16_t 		Y;
} POINT_t;

#define x1 96
#define x2 228
#define y1 27
#define y2 153

// Public Function Prototypes

MapSection FindStraightSection(POINT_t current);
MapDecisionZone FindDecisionZone( POINT_t current );


ES_Event RunDriving( ES_Event CurrentEvent );
void StartDriving ( ES_Event CurrentEvent );
DrivingState QueryDriving ( void );

#endif /*Driving_H */

