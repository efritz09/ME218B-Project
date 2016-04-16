/****************************************************************************
 Header file for obstacle state machine

 ****************************************************************************/

#ifndef Obstacle_H
#define Obstacle_H

// State definitions for use with the query function
typedef enum { Orient, Turning, FindX, FindY,
								DrivingToTeeterPoint,PauseAtTeeter, ExitingObstacle } ObstacleState ;

// Public Function Prototypes

ES_Event RunObstacle( ES_Event CurrentEvent );
void StartObstacle ( ES_Event CurrentEvent );
void DriveStraight(int pwr, uint16_t time);
ObstacleState QueryObstacle ( void );

#endif /*Obstacle_H */

