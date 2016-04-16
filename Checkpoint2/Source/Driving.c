/****************************************************************************
 Module
   Driving.c

 Revision			Revised by: 
	0.1.1				Alex

 Description
	Driving state machine that controls the driving

 Edits:
	0.1.1 - Set up as template 
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "Drive.h"
#include "SPITemplate.h"
#include "PWM.h"
#include "GamePlay.h"
#include "Master.h"
#include "Driving.h"
#include "Shooting.h"
#include "Obstacle.h"
#include <stdio.h>

/*----------------------------- Module Defines ----------------------------*/
// define constants for this machine


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/
static ES_Event DuringAtPositionState( ES_Event Event);
static ES_Event DuringMovingState( ES_Event Event);
static POINT_t FindNextPoint ( POINT_t current);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static DrivingState CurrentState;

// Define pertinant points
/*
const POINT_t point1 = {0,0};
const POINT_t point2 = {1,0};
const POINT_t point3 = {2,0};
const POINT_t point4 = {3,0};
const POINT_t point5 = {4,0};
const POINT_t point6 = {5,0};
*/

// Create matrix of points
uint16_t pointMatrix_X[6] = {0, 1, 2, 3, 4, 5};
uint16_t pointMatrix_Y[6] = {0, 1, 2, 3, 4, 5};

POINT_t currentPoint;
POINT_t nextPoint;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
    RunDriving

 Parameters
   unsigned char: the event to process

 Returns
   unsigned char: an event to return

 Description
   implements driving state machine using DRS data
****************************************************************************/
ES_Event RunDriving( ES_Event CurrentEvent )
{
   bool MakeTransition = false;/* are we making a state transition? */
   DrivingState NextState = CurrentState;
   ES_Event ReturnEvent = CurrentEvent; // assume we are not consuming event
	
   //printf("Event posted to driving \r\n");

   switch ( CurrentState )
   {
       case AtPositionState :
         // Execute during function
         CurrentEvent = DuringAtPositionState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case NextPointCalculated: //If event is event one
				  printf("Next Point Calculated \r\n");
                  NextState = MovingState;//Decide what the next state will be
                  MakeTransition = true; //mark that we are taking a transition
                  break;
            }
         }
         break;
        case MovingState :
         // Execute during function
         CurrentEvent = DuringMovingState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case AtNextPoint: //If event is event one
				  printf("At Next Point \r\n");
                  NextState = AtPositionState;//Decide what the next state will be
                  MakeTransition = true; //mark that we are taking a transition
                  break;
            }
         }
         break;
    }
    //   If we are making a state transition
    if (MakeTransition == true)
    {
       //   Execute exit function for current state
       CurrentEvent.EventType = ES_EXIT;
       RunDriving(CurrentEvent);

       CurrentState = NextState; //Modify state variable

       //   Execute entry function for new state
       CurrentEvent.EventType = ES_ENTRY;
       RunDriving(CurrentEvent);
     }
     return(ReturnEvent);
}
/****************************************************************************
 Function
     StartDriving

 Parameters
     Event_t CurrentEvent

 Returns
     None

 Description
     Starts in at position state
****************************************************************************/
void StartDriving ( ES_Event CurrentEvent )
{
	// local variable to get debugger to display the value of CurrentEvent
	ES_Event LocalEvent = CurrentEvent;
	CurrentState = AtPositionState;
	
	// Make current positon acutual position accordign to DRS
	KART_t myKart = QueryMyKart( );
	currentPoint.X = myKart.KartX;
	currentPoint.Y = myKart.KartY;
	
   // Call the entry function (if any) for the ENTRY_STATE
   RunDriving(CurrentEvent);
}

/****************************************************************************
 Function
     QueryDriving

 Parameters
     None

 Returns
     unsigned char The current state of the Template state machine

 Description
     returns the current state of the driving state machine
****************************************************************************/
DrivingState QueryDriving ( void )
{
   return(CurrentState);
}

/***************************************************************************
 private functions
 ***************************************************************************/

static ES_Event DuringAtPositionState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY) {
		printf("Entered At Position State \r\n");
        // No entry functionality 
    }
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited At Position State \r\n");
        // No exit functionality 
    }
	else {
		// Calculate current point
		KART_t myKart = QueryMyKart( );
		currentPoint.X = myKart.KartX;
		currentPoint.Y = myKart.KartY;
		
		// Find next point based on current point
		nextPoint = FindNextPoint(currentPoint);
		printf("Find Next Point (And Post) \r\n");
		// Post calculation event
		ES_Event newEvent = {NextPointCalculated, 0};
		PostMaster(newEvent);
    }
    return( Event );  // Don't remap event
}

static ES_Event DuringMovingState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY) {
		printf("Entered Moving State \r\n");
        printf("Drive To Next Point \r\n");
		// Drive to next point
		DriveTo(nextPoint.X, nextPoint.Y);
    }
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Moving State \r\n");
        // No exit functionality 
    }
	else {
		// No during functionality
    }
    return( Event );  // Don't remap event
}

static POINT_t FindNextPoint( POINT_t current ) {
	// find correct point according to current location
	return current;
}

