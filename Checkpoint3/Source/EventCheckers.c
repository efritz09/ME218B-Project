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
	  ES_Event newEvent = {DetectedLeftBeacon, 0};
      PostMaster( newEvent );
	  printf("POST - Detected Left Beacon \r\n");
    }
	else if ( ThisEvent.EventParam == '9'){
	  ES_Event newEvent = {DetectedRightBeacon, 0};
      PostMaster( newEvent );
	  printf("POST - Detected Right Beacon \r\n");
    }
	else if ( ThisEvent.EventParam == 'q'){
	  ES_Event newEvent = {BallFired, 0};
      PostMaster( newEvent );
	  printf("POST - Ball Fired \r\n");
    }
	else if ( ThisEvent.EventParam == 'w'){
	  ES_Event newEvent = {MiddleTapeTripped, 0};
      PostMaster( newEvent );
	  printf("POST - Middle Tape Tripped \r\n");
    }
	else if ( ThisEvent.EventParam == 'e'){
	  ES_Event newEvent = {MiddleTapeLost, 0};
      PostMaster( newEvent );
	  printf("POST - Middle Tape Lost \r\n");
    }
	else if ( ThisEvent.EventParam == 'r'){
	  ES_Event newEvent = {RightTapeTripped, 0};
      PostMaster( newEvent );
	  printf("POST - Right Tape Tripped \r\n");
    }
	else if ( ThisEvent.EventParam == 't'){
	  ES_Event newEvent = {LeftTapeTripped, 0};
      PostMaster( newEvent );
	  printf("POST - Left Tape Tripped \r\n");
    }
	else if ( ThisEvent.EventParam == 'y'){
	  ES_Event newEvent = {ObstacleTraversed, 0};
      PostMaster( newEvent );
	  printf("POST - Obstacle Traversed \r\n");
    }
	else if ( ThisEvent.EventParam == 'u'){
	  ES_Event newEvent = {CautionOver, 0};
      PostMaster( newEvent );
	  printf("POST - Caution Over \r\n");
    }
	else if ( ThisEvent.EventParam == 'i'){
	  ES_Event newEvent = {GameOver, 0};
      PostMaster( newEvent );
	  printf("POST - Game Over \r\n");
    }
	else if ( ThisEvent.EventParam == 'o'){
	  ES_Event newEvent = {EmergencyStop, 0};
      PostMaster( newEvent );
	  printf("POST - Emergency Stop \r\n");
    }
	else if ( ThisEvent.EventParam == 'z'){
	  ES_Event newEvent = {ChangeOpMode, 0};
      PostMaster( newEvent );
	  printf("POST - Change Operation Mode \r\n");
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
	else{   // otherwise post to Service 0 for processing
      PostMaster( ThisEvent );
		printf("POST - Unknown key \r\n");
    }
    return true;
  }
  return false;
}
