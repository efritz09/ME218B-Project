#include "Headers.h"

// Find the straightway that the bot is currently on
MapSection FindSection( POINT_t current ) {
	
	KART_t myKart;
	myKart = QueryMyKart( );
	
	if(current.Y >= SDZ_Ymin && current.Y <= SDZ_Ymax && current.X <= SDZ_X && !myKart.ShotComplete) {
		printf("In Shooting Decision Zone \r\n");
		return ShootingDecisionZone;
	}
	// Obstacle Decision Zone
	else if(current.Y >= ODZ_Y && current.X >= ODZ_Xmin && current.X <= ODZ_Xmax && !myKart.ObstacleComplete) {
		printf("In Obstacle Decision Zone \r\n");
		return  ObstacleDecisionZone;
	}
	// Find section of map according to current location
	else if((current.Y >= y2 && current.X <= x2) || ((current.Y >= y2-cornerSize && current.Y <= y2) && (current.X >= x1 && current.X <= x1+cornerSize))) {
		printf("In Bottom Straight \r\n");
		return BottomStraight;
	}
	else if((current.Y >= y1 && current.X >= x2) || ((current.Y >= y2-cornerSize && current.Y <= y2) && (current.X >= x2-cornerSize && current.X <= x2))) {
		printf("In Right Straight \r\n");
		return RightStraight;
	}
	else if((current.Y <= y1 && current.X >= x1) || ((current.Y >= y1 && current.Y <= y1+cornerSize) && (current.X >= x2-cornerSize && current.X <= x2))) {
		printf("In Top Straight \r\n");
		return TopStraight;
	}
	else if((current.Y <= y2 && current.X <= x1) || ((current.Y >= y1 && current.Y <= y1+cornerSize) && (current.X >= x1 && current.X <= x1+cornerSize))) {
		printf("In Left Straight \r\n");
		return LeftStraight;
	}
	else if(current.Y >= y2 - cornerSize) {
		printf("In Bin \r\n");
		return Bin;
	}
	else if(current.X >= x2 - cornerSize) {
		printf("In Rin \r\n");
		return Rin;
	}
	else if(current.Y <= y1 + cornerSize) {
		printf("In Tin \r\n");
		return Tin;
	}
	else if(current.X <= x1 + cornerSize) {
		printf("In Lin \r\n");
		return Lin;
	}
	else{
			return DeadZone;
	}
}

// Find the straightway that the bot is currently on
MapSection FindNextSection( MapSection current ) {
	printf("Next Section is ");
	switch(current) {
		case BottomStraight:
			printf("Right Straight \r\n");
			return RightStraight;
		case RightStraight: 
			printf("Top Straight \r\n");
			return TopStraight;
		case TopStraight:
			printf("Left Straight \r\n");
			return LeftStraight;
		case LeftStraight:
			printf("Bottom Straight \r\n");
			return BottomStraight;	
		default:
			return DeadZone;
	}
}


