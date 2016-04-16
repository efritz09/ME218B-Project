/****************************************************************************
 
  Header file for the points module

 ****************************************************************************/

#ifndef Points_H
#define Points_H

#include "Headers.h"

// State definitions for use with the query function
typedef enum {BottomStraight, RightStraight, TopStraight, LeftStraight, ShootingDecisionZone, ObstacleDecisionZone, DeadZone} MapSection;

// Define point structure
typedef struct {
			uint16_t 		X;
			uint16_t 		Y;
} POINT_t;

/*----------------------- Public Function Prototypes ----------------------*/
MapSection FindSection( POINT_t current );
MapSection FindNextSection( MapSection current );

// Define inner square lines
#define x1 115
#define x2 210
#define y1 45
#define y2 125

// Define size of corner regions that are added to straightaways
#define cornerSize 15

// Define pertinant points

// Bottom right waypoint (actually top left)
#define BR_X 230
#define BR_Y 160

// Top right waypoint (actually bottom left)
#define TR_X 230
#define TR_Y 17

// Top left waypoint (actually bottom right)
#define TL_X 100
#define TL_Y 17

// Bottom left waypoint (actually top right)
#define BL_X 100
#define BL_Y 153

// Shooting point
#define SP_X 133
#define SP_Y 84

// Obstacle entry point
#define OE_X 181
#define OE_Y 140

// Shooting decision zone
#define SDZ_Ymin 64
#define SDZ_Ymax 100
#define SDZ_X 130

// Obstacle decision zone
#define ODZ_Xmin 160
#define ODZ_Xmax 181
#define ODZ_Y 135

// Obstacle Centerline
#define X_O 181
// Obstacle entrance point
#define Y_O 140
// Obstacle Teetering Point
#define Y_teeter 84


#endif /* Points_H */
