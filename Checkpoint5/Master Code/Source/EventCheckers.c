/****************************************************************************
 Module
   EventCheckers.c

 Revision
   1.0.1 

 Description
   Keystroke events to simulate the Command Generator's commands

 Notes
   
****************************************************************************/

// this will pull in the symbolic definitions for events, which we will want
// to post in response to detecting events
#include "ES_Configure.h"
// this will get us the structure definition for events, which we will need
// in order to post events in response to detecting events
#include "ES_Events.h"
// if you want to use distribution lists then you need those function 
// definitions too.
#include "ES_PostList.h"
// This include will pull in all of the headers from the service modules
// providing the prototypes for all of the post functions
#include "ES_ServiceHeaders.h"
// this test harness for the framework references the serial routines that
// are defined in ES_Port.c
#include "ES_Port.h"
// include our own prototypes to insure consistency between header & 
// actual functionsdefinition
#include "EventCheckers.h"
#include "Headers.h"

#define PI 3.14159265
/****************************************************************************
 Function
   Check4Keystroke
 Parameters
   None
 Returns
   bool: true if a new key was detected & posted
 Description
   checks to see if a new key from the keyboard is detected and, if so, 
   posts the appropriate command to the SPI service
 Notes
   The functions that actually check the serial hardware for characters
   and retrieve them are assumed to be in ES_Port.c
   Since we always retrieve the keystroke when we detect it, thus clearing the
   hardware flag that indicates that a new key is ready this event checker 
   will only generate events on the arrival of new characters, even though we
   do not internally keep track of the last keystroke that we retrieved.
 Author
   J. Edward Carryer, 08/06/13, 13:48
****************************************************************************/
bool Check4Keystroke(void)
{
  if ( IsNewKeyReady() ) // new key waiting?
  {
    ES_Event ThisEvent;
    ThisEvent.EventType = ES_NEW_KEY;
    ThisEvent.EventParam = GetNewKey();
    // test distribution list functionality by sending the 'L' key out via
    // a distribution list.
    if ( ThisEvent.EventParam == '1'){
	  ES_Event newEvent = {InShootingDecisionZone, 0};
      PostMaster( newEvent );
	  printf("POST - In Shooting Decision Zone \r\n");
    }
	else if ( ThisEvent.EventParam == '2'){
	 ES_Event newEvent = {InObstacleDecisionZone, 0};
     PostMaster( newEvent );
	 printf("POST - In Obstacle Decision Zone \r\n");
    }
	else if ( ThisEvent.EventParam == '3'){
	  ES_Event newEvent = {CautionFlagDropped, 0};
      PostMaster( newEvent );
	  printf("POST - Caution Flag Dropped \r\n");
    }
	else if ( ThisEvent.EventParam == '4'){
      ES_Event newEvent = {FlagDropped, 0};
      PostMaster( newEvent );
	  printf("POST - Flag Dropped \r\n");
    }
	else if ( ThisEvent.EventParam == '5'){
	  ES_Event newEvent = {NextPointCalculated, 0};
      PostMaster( newEvent );
	  printf("POST - Next Point Calculated \r\n");
    }
	else if ( ThisEvent.EventParam == '6'){
	  ES_Event newEvent = {AtNextPoint, 0};
      PostMaster( newEvent );
	  printf("POST - At Next Point \r\n");
    }
	else if ( ThisEvent.EventParam == '7'){
	  ES_Event newEvent = {AtNextAngle, 0};
      PostMaster( newEvent );
	  printf("POST - At Next Angle \r\n");
    }
	else if ( ThisEvent.EventParam == '8'){
	  ES_Event newEvent = {DetectedBeacon, 0};
      PostMaster( newEvent );
	  printf("POST - Detected Beacon \r\n");
    }
	else if ( ThisEvent.EventParam == '9'){
	  ES_Event newEvent = {ObstacleTraversed, 0};
      PostMaster( newEvent );
	  printf("POST - Obstacle Traversed \r\n");
    }
	else if ( ThisEvent.EventParam == '0'){
	  ES_Event newEvent = {GameOver, 0};
      PostMaster( newEvent );
	  printf("POST - Game Over \r\n");
    }
	else if ( ThisEvent.EventParam == 'q'){
	  ES_Event newEvent = {EmergencyStop, 0};
      PostMaster( newEvent );
	  printf("POST - Emergency Stop \r\n");
    }
	else if ( ThisEvent.EventParam == 'w'){
	  ES_Event newEvent = {SpeedChangePort, 10};
      PostMaster( newEvent );
	  printf("POST - Speed Change Port \r\n");
    }
	else if ( ThisEvent.EventParam == 'e'){
	  ES_Event newEvent = {SpeedChangeStarboard, 10};
      PostMaster( newEvent );
	  printf("POST - Speed Change Starboard \r\n");
    }
	else if ( ThisEvent.EventParam == 'r'){
	  ES_Event newEvent = {AtRatio, 0};
      PostMaster( newEvent );
	  printf("POST - Tape Sensors At Correct Ratio \r\n");
    }
	else if ( ThisEvent.EventParam == 't'){
	  ES_Event newEvent = {OffRatio, 0};
      PostMaster( newEvent );
	  printf("POST - Tape Sensors Off Correct Ratio \r\n");
    }
	else if ( ThisEvent.EventParam == 'y'){
	  ES_Event newEvent = {ToDriving, 0};
      PostMaster( newEvent );
	  printf("POST - Move To Driving SM \r\n");
    }
	else if ( ThisEvent.EventParam == 'u'){
	  ES_Event newEvent = {ToShooting, 0};
      PostMaster( newEvent );
	  printf("POST - Move To Shooting SM \r\n");
    }
	else if ( ThisEvent.EventParam == 'i'){
	  ES_Event newEvent = {ToObstacle, 0};
      PostMaster( newEvent );
	  printf("POST - Move To Obstacle SM \r\n");
    }
	else if ( ThisEvent.EventParam == 'o'){
	  ES_Event newEvent = {AtNextPoint, 1};
      PostMaster( newEvent );
	  printf("POST - At Next Point \r\n");
    }
	else if ( ThisEvent.EventParam == 'x'){
	  ES_Event newEvent = {Waypoint_BR, 0};
      PostMaster( newEvent );
	  printf("POST - Bottom Right Waypoint \r\n");
    }
	else if ( ThisEvent.EventParam == 'c'){
	  ES_Event newEvent = {Waypoint_TR, 0};
      PostMaster( newEvent );
	  printf("POST - Top Right Waypoint \r\n");
    }
	else if ( ThisEvent.EventParam == 'v'){
	  ES_Event newEvent = {Waypoint_TL, 0};
      PostMaster( newEvent );
	  printf("POST - Top Left Waypoint \r\n");
    }
	else if ( ThisEvent.EventParam == 'b'){
	  ES_Event newEvent = {Waypoint_BL, 0};
      PostMaster( newEvent );
	  printf("POST - Bottom Left Waypoint \r\n");
    }
	else if ( ThisEvent.EventParam == 'n'){
	  ES_Event newEvent = {Waypoint_S, 0};
      PostMaster( newEvent );
	  printf("POST - Shooting Waypoint \r\n");
    }
	else if ( ThisEvent.EventParam == 'm'){
	  ES_Event newEvent = {Waypoint_O, 0};
      PostMaster( newEvent );
	  printf("POST - Obstacle Entry Waypoint \r\n");
    }
	else if ( ThisEvent.EventParam == 'p'){
		float desiredX = 120;
		float desiredY = 88;
	
		// Query DRS for current postion, angle
		KART_t myKart = QueryMyKart( );
	
		// Find next section
		uint16_t currentX = myKart.KartX;
		uint16_t currentY = myKart.KartY;
	
		printf("     Data X: %d Y: %d Theta: %d \n\r", myKart.KartX, myKart.KartY, myKart.KartTheta);
	
		// Find distance to travel
		float deltaX = desiredX - myKart.KartX;
		float deltaY = desiredY - myKart.KartY; 
		float dist = sqrt(pow(deltaX,2) + pow(deltaY,2));
		printf("     deltaX: %f deltaY: %f\n\r", deltaX, deltaY);
	
		// Test values
		float absDeltaX = deltaX;
		float absDeltaY = deltaY;
	
		if(deltaX < 0) {
			absDeltaX = (-1)*deltaX;
		}
		if(deltaY < 0) {
			absDeltaY = (-1)*deltaY;
		}
	
		// Calculate angle
		int desiredTheta = abs(atan((absDeltaY)/(absDeltaX)) * 180 / PI); 
		//printf("Arctan Output: %d \n\r", abs(atan(abs(142/)) * 180 / PI));
		printf("Arctan Output: %d \n\r", desiredTheta);
	
		/* Calculate the desired theta desired based on the 
		left handed coordiante system and right handed theta*/
		if(deltaX < 0 && deltaY < 0) {
			desiredTheta = (-1)*desiredTheta;
		}
		else if(deltaX > 0 && deltaY > 0) {
			desiredTheta = 180 - desiredTheta;
		}
		else if(deltaX > 0 && deltaY < 0) {
			desiredTheta = -180 +desiredTheta;
		}
		// Combat case where deltaY is exactly zero
		else if(deltaX > 0 && deltaY == 0) {
			desiredTheta = 180;
		}
		else if(deltaX < 0 && deltaY == 0) {
			desiredTheta = 0;
		}	
	
		// Garuntee that the desired theta is between 0 and 360
		if(desiredTheta < 0) {
			desiredTheta = 360 + desiredTheta;
		}
	
		// Ignore arctan calculated desired theta value if deltaX is zero as this will 
		// lead to a garbage reading from arctan
		if(deltaX == 0) {
			if(deltaY > 0) {
				desiredTheta = 90;
			}
			if(deltaY < 0) {
			desiredTheta = 270;
			}
		}
		// Store value of desiredTheta
		uint16_t lastCommandedTheta = desiredTheta;
	
		// Find differnece in angles
		int deltaTheta;
		deltaTheta = desiredTheta - myKart.KartTheta; //getDesiredTheta(); 
	
		//garuntee that the bot never rotates more than 180 degrees
		if(deltaTheta > 180) {
			deltaTheta = deltaTheta - 360;
		}
		if(deltaTheta < (-1)*180) {
			deltaTheta = deltaTheta + 360;
		}
		printf("Vector Calculations Dist: %f  Desired Theta: %d DeltaTheta: %d\n\n\n\r", dist, desiredTheta, deltaTheta);
  }
	else{   // otherwise post to Service 0 for processing
      PostMaster( ThisEvent );
		printf("POST - Unknown key \r\n");
    }
    return true;
  }
  return false;
}
