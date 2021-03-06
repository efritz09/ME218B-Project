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
#include "RunningGame.h"
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
static POINT_t FindNextPoint( POINT_t current );
MapSection FindSection( POINT_t current );

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static DrivingState CurrentState;
static bool keyControl = false;/* set true to control with keystrokes*/

// Define pertinant points
/*
POINT_t point1 = {232,160}; bottom right corner
POINT_t point2 = {232,32}; top right corner
POINT_t point3 = {232,32}; top left corner
POINT_t point4 = {60,32}; bottom left corner
*/

// Create matrix of points
uint16_t cornerPointMatrix_X[4] = {232, 232, 60, 60};
uint16_t cornerPointMatrix_Y[4] = {160, 32, 32, 160};

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
	
	/*switching between keyboard control and automated control*/
   if(CurrentEvent.EventType == ChangeOpMode) {
	   keyControl = !keyControl;
	   if(keyControl == true) {
		   printf("Keyboard Control Activated \r\n");
	   }
	   else {
		   printf("Algorithim Control Activated \r\n");
		   //nextPoint = FindNextPoint(currentPoint);
	   }
   }
	
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
							 
					/*If we are controlling with key strokes we can set the next point*/
			   case Waypoint_BR: 
				  if(keyControl) {
					printf("Bottom Right Waypoint Selected \r\n");
					nextPoint.X = cornerPointMatrix_X[0];
					nextPoint.Y = cornerPointMatrix_Y[0];
					// Post calculation event
					ES_Event newEvent = {NextPointCalculated, 0};
					PostMaster(newEvent);
				  }
                  break;
			   case Waypoint_TR:
				  if(keyControl) {
					printf("Top Right Waypoint Selected \r\n");
					nextPoint.X = cornerPointMatrix_X[1];
					nextPoint.Y = cornerPointMatrix_Y[1];
					// Post calculation event
					ES_Event newEvent = {NextPointCalculated, 0};
					PostMaster(newEvent);
				  }
                  break;
			   case Waypoint_TL:
				  if(keyControl) {
					printf("Top Left Waypoint Selected \r\n");
					nextPoint.X = cornerPointMatrix_X[2];
					nextPoint.Y = cornerPointMatrix_Y[2];
					// Post calculation event
					ES_Event newEvent = {NextPointCalculated, 0};
					PostMaster(newEvent);
				  }
                  break;
			   case Waypoint_BL: 
				  if(keyControl) {
					printf("Bottom Left Waypoint Selected \r\n");
					nextPoint.X = cornerPointMatrix_X[3];
					nextPoint.Y = cornerPointMatrix_Y[3];
					// Post calculation event
					ES_Event newEvent = {NextPointCalculated, 0};
					PostMaster(newEvent);
				  }
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
	//ES_Event LocalEvent = CurrentEvent;
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
        
		
		// Calculate current point
		KART_t myKart = QueryMyKart( );
		currentPoint.X = myKart.KartX;
		currentPoint.Y = myKart.KartY;
			
		printf("%d %d %d \n\r", myKart.KartX, myKart.KartY, myKart.KartTheta);
		// statement to print where we 'think' we are
			FindSection(currentPoint);
		
		// Find next point based on current point
		printf("Find Next Point\r\n");
		if(!keyControl) {
			nextPoint = FindNextPoint(currentPoint);
		}
		else {
			printf("Enter Desired Waypoint\r\n");
		}
    }
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited At Position State \r\n");
        // No exit functionality 
    }
	else {
		// No during functionallity	
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
	// Create return variable
	POINT_t returnPoint;
	// Find section in map where we think we are located
	MapSection currentSection = FindSection(current);

	/*depending on the section of the map where 
	we are located pick the next location to drive*/
	switch (currentSection)
	{
			// set the location via the 4 coners that define a lap
		 case BottomStraight:
			printf("Next Waypoint Is Bottom Right \r\n");
			returnPoint.X = cornerPointMatrix_X[0];
			returnPoint.Y = cornerPointMatrix_Y[0];
			break;
		 case RightStraight:
			printf("Next Waypoint Is Top Right \r\n");
			returnPoint.X = cornerPointMatrix_X[1];
			returnPoint.Y = cornerPointMatrix_Y[1];
			break;
		 case TopStraight:
			printf("Next Waypoint Is Top Left \r\n");
			returnPoint.X = cornerPointMatrix_X[2];
			returnPoint.Y = cornerPointMatrix_Y[2];
			break;
		 case LeftStraight:
			printf("Next Waypoint Is Bottom Left \r\n");
			returnPoint.X = cornerPointMatrix_X[3];
			returnPoint.Y = cornerPointMatrix_Y[3];
			break;
	}
	
	// Post calculation event
	ES_Event newEvent = {NextPointCalculated, 0};
	PostMaster(newEvent);
	
	return returnPoint;
}

MapSection FindSection( POINT_t current ) {
	// Create return map section
	MapSection returnMapSection;
	// Find section of map according to current location
	if( (current.Y >= 150 && current.X <= 222) || ((current.Y >= 96 && current.Y <= 150) && (current.X >= 70 && current.X <= 146))) {
		printf("In Bottom Straight \r\n");
		returnMapSection = BottomStraight;
	}
	else if((current.Y >= 42 && current.X >= 222) || ((current.Y >= 96 && current.Y <= 150) && (current.X >= 146 && current.X <= 222))) {
		printf("In Right Straight \r\n");
		returnMapSection = RightStraight;
	}
	else if((current.Y <= 42 && current.X >= 70) || ((current.Y >= 42 && current.Y <= 96) && (current.X >= 146 && current.X <= 222))) {
		printf("In Top Straight \r\n");
		returnMapSection = TopStraight;
	}
	else if((current.Y <= 150 && current.X <= 70)  || ((current.Y >= 42 && current.Y <= 96) && (current.X >= 70 && current.X <= 146))) {
		printf("In Left Straight \r\n");
		returnMapSection = LeftStraight;
	}
		
	return returnMapSection;
}
