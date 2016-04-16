/****************************************************************************
 
  Header file for the points module

 ****************************************************************************/

#ifndef Points_H
#define Points_H

#include "Headers.h"

// State definitions for use with the query function
typedef enum {BottomStraight, RightStraight, TopStraight, LeftStraight, ShootingDecisionZone, ObstacleDecisionZone, DeadZone, Bin, Rin, Tin, Lin} MapSection;

// Define point structure
typedef struct {
			uint16_t 		X;
			uint16_t 		Y;
} POINT_t;

/*----------------------- Public Function Prototypes ----------------------*/
MapSection FindSection( POINT_t current );
MapSection FindNextSection( MapSection current );

// Define inner square lines
#define x1 96
#define x2 222
#define y1 29
#define y2 154

// Define size of corner regions that are added to straightaways
#define cornerSize 15

// Define pertinant points

// Bottom right waypoint (actually top left)
#define BR_X 237
#define BR_Y 155

// Top right waypoint (actually bottom left)
#define TR_X 237
#define TR_Y 30

// Top left waypoint (actually bottom right)
#define TL_X 83
#define TL_Y 30

// Bottom left waypoint (actually top right)
#define BL_X 84
#define BL_Y 155

// Shooting point
#define SP_X 107
#define SP_Y 80

// Obstacle entry point
#define OE_X 184
#define OE_Y 162

// Shooting decision zone
#define SDZ_Ymin 50
#define SDZ_Ymax 107
#define SDZ_X 160

// Obstacle decision zone
#define ODZ_Xmin 137
#define ODZ_Xmax 175
#define ODZ_Y 120

// Obstacle Centerline
#define X_O 174
// Obstacle entrance point
#define Y_O 145
// Obstacle Teetering Point
#define Y_teeter 81


#endif /* Points_H */
