/****************************************************************************
 Module
   MapKeys.c

 Revision
   1.0.1

 Description
   This service maps keystrokes to events for the microwave oven.
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
#include <stdio.h>
#include <ctype.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "Master.h"


/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitMapKeys

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any 
     other required initialization for this service
 Notes

 Author
     J. Edward Carryer, 02/07/12, 00:04
****************************************************************************/
bool InitMapKeys ( uint8_t Priority )
{
	
  printf("Keys Initialized \r\n");
  MyPriority = Priority;

  return true;
}

/****************************************************************************
 Function
     PostMapKeys

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostMapKeys( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}


/****************************************************************************
 Function
    RunMapKeys

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   maps keys to Events for HierMuWave Example
 Notes
   
 Author
   J. Edward Carryer, 02/07/12, 00:08
****************************************************************************/
ES_Event RunMapKeys( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    if ( ThisEvent.EventType == ES_NEW_KEY) // there was a key pressed
    {
        switch (ThisEvent.EventParam)
        {
			printf("KEYHIT \r\n");
			ES_Event ThisEvent;
			ThisEvent.EventType = ES_NEW_KEY;
			ThisEvent.EventParam = GetNewKey();
			// test distribution list functionality by sending the 'L' key out via
			// a distribution list.
			if ( ThisEvent.EventParam == '1'){
			  ES_Event newEvent = {InShootingDecisionZone, 0};
			  PostMaster( newEvent );
			  printf("POST1 \r\n");
			}
			else if ( ThisEvent.EventParam == '2'){
			 ES_Event newEvent = {InObstacleDecisionZone, 0};
			 PostMaster( newEvent );
			}
			else if ( ThisEvent.EventParam == '3'){
			  ES_Event newEvent = {CautionFlagDropped, 0};
			  PostMaster( newEvent );
			}
			else if ( ThisEvent.EventParam == '4'){
			  ES_Event newEvent = {FlagDropped, 0};
			  PostMaster( newEvent );
			}
			else if ( ThisEvent.EventParam == '5'){
			  ES_Event newEvent = {NextPointCalculated, 0};
			  PostMaster( newEvent );
			}
			else if ( ThisEvent.EventParam == '6'){
			  ES_Event newEvent = {AtNextPoint, 0};
			  PostMaster( newEvent );
			}
			else if ( ThisEvent.EventParam == '7'){
			  ES_Event newEvent = {AtNextAngle, 0};
			  PostMaster( newEvent );
			}
			else if ( ThisEvent.EventParam == '8'){
			  ES_Event newEvent = {DetectedLeftBeacon, 0};
			  PostMaster( newEvent );
			}
			else if ( ThisEvent.EventParam == '9'){
			  ES_Event newEvent = {DetectedRightBeacon, 0};
			  PostMaster( newEvent );
			}
			else if ( ThisEvent.EventParam == 'e'){
			  ES_Event newEvent = {BallFired, 0};
			  PostMaster( newEvent );
			}
			else if ( ThisEvent.EventParam == 'f'){
			  ES_Event newEvent = {MiddleTapeTripped, 0};
			  PostMaster( newEvent );
			}
			else if ( ThisEvent.EventParam == 'a'){
			  ES_Event newEvent = {MiddleTapeLost, 0};
			  PostMaster( newEvent );
			}
			else if ( ThisEvent.EventParam == 'y'){
			  ES_Event newEvent = {RightTapeTripped, 0};
			  PostMaster( newEvent );
			}
			else if ( ThisEvent.EventParam == 'l'){
			  ES_Event newEvent = {LeftTapeTripped, 0};
			  PostMaster( newEvent );
			}
			else if ( ThisEvent.EventParam == 'p'){
			  ES_Event newEvent = {ObstacleTraversed, 0};
			  PostMaster( newEvent );
			}
			else{   // otherwise post to Service 0 for processing
			  PostMaster( ThisEvent );
			}
        }
	}
  return ReturnEvent;
}