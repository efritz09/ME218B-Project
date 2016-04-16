/****************************************************************************
 Module
   Driving.c

 Revision			Revised by: 
	0.1.1				Alex

 Description
	Driving state machine that controls the driving

 Edits:
	0.1.1 - Set up as template 
	0.1.2 - Separated moving state into a turning state and driving state
	0.2.1 - Added all possible waypoints
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "Headers.h"

/*----------------------------- Module Defines ----------------------------*/
// define constants for this machine


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/
static ES_Event DuringAtPositionState( ES_Event Event );
static ES_Event DuringGeneratePathState( ES_Event Event );
static ES_Event DuringTurningState( ES_Event Event );
static ES_Event DuringDrivingForwardState( ES_Event Event );

static POINT_t FindNextPoint( POINT_t current );

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static DrivingState CurrentState;

POINT_t currentPoint;
POINT_t nextPoint;

static int exitZone = 0;
static bool notShot = true;
static bool notObs = true;

KART_t myKart;

// Define pertinant points
/*
POINT_t point1 = {245,172}; bottom right corner
POINT_t point2 = {245,7}; top right corner
POINT_t point3 = {80,7}; top left corner
POINT_t point4 = {72,172}; bottom left corner
*/

// Create matrix of points for corner waypoints
uint16_t cornerPointMatrix_X[4] = {BR_X, TR_X, TL_X, BL_X};
uint16_t cornerPointMatrix_Y[4] = {BR_Y, TR_Y, TL_Y, BL_Y};

// Shooting and obstacle waypoints
POINT_t ObstacleEntry = {OE_X,OE_Y};
POINT_t ShootingPoint = {SP_X,SP_Y};

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
	 
	 // Calculate current point
		myKart = QueryMyKart( );
		currentPoint.X = myKart.KartX;
		currentPoint.Y = myKart.KartY;

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
								NextState = GeneratePathState;//Decide what the next state will be
								MakeTransition = true; //mark that we are taking a transition
                break;							 
            }
         }
         break;
				case GeneratePathState :
         // Execute during function
         CurrentEvent = DuringGeneratePathState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case PathGenerated:
									printf("Path Generated \r\n");
									// Move within the driving SM
									NextState = TurningState;
									MakeTransition = true;
									break;
            }
         }
				 break;
        case TurningState :
         // Execute during function
         CurrentEvent = DuringTurningState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case AtNextAngle:
									printf("At Next Angle \r\n");
									// Move within the driving SM
									NextState = DrivingForwardState;
									MakeTransition = true;
									break;
            }
         }
         break;
				case DrivingForwardState :
         // Execute during function
         CurrentEvent = DuringDrivingForwardState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case AtNextPoint:
									printf("At Next Point \r\n");

									// Move within the driving SM
									NextState = AtPositionState;
									MakeTransition = true;
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
	
	// Make current positon acutual position according to DRS
	KART_t myKart = QueryMyKart( );
	currentPoint.X = myKart.KartX;
	currentPoint.Y = myKart.KartY;
	
	exitZone = 0;
	
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

static ES_Event DuringAtPositionState( ES_Event Event )
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY  ) {
		printf("Entered At Position State \r\n");
				
			printf("%d %d %d \n\r", myKart.KartX, myKart.KartY, myKart.KartTheta);
			
			// Find next point based on current point and game state
			printf("Find Next Point\r\n");
			nextPoint = FindNextPoint(currentPoint);
			// Post calculation event
			ES_Event newEvent = {NextPointCalculated, 0};
			PostMaster(newEvent);
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

static ES_Event DuringGeneratePathState( ES_Event Event )
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY  ) {
		printf("Entered Generate Path State \r\n");
     // Call calulate funciton with current point
		 if(exitZone == 0) {
			Calculate( nextPoint.X, nextPoint.Y );
		 }
		 else {
			 ExitZoneCalc(exitZone);
			 exitZone = 0;
		 }
			
    }
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Generate Path State \r\n");
        // No exit functionality 
    }
	else {
		// No during functionality
    }
    return( Event );  // Don't remap event
}

static ES_Event DuringTurningState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY  ) {
			printf("Entered Turning State \r\n");
				 
			// Turn to next point	
			printf("Turn To Next Point \r\n");
			TurnTheta( );
    }
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Turning State \r\n");
        // No exit functionality 
    }
	else {
		// No during functionality
    }
    return( Event );  // Don't remap event
}

static ES_Event DuringDrivingForwardState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY  ) {
			printf("Entered Driving Forward State \r\n");
			printf("Drive Forward \r\n");
			// Drive to next point
			DriveForward( );
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
	
	switch (currentSection)
	{
		  // First check obstacle and shooting decision zones
			case ShootingDecisionZone:
			if(!myKart.ShotComplete && notShot) {
				printf("Move to Shooting SM\r\n");

				ES_Event newEvent = {ToShooting, 0};
				PostMaster(newEvent);
				
				notShot = false;
				
				return ShootingPoint;
			}
			else {
				POINT_t returnPoint;
				returnPoint.X = cornerPointMatrix_X[3];
				returnPoint.Y = cornerPointMatrix_Y[3];
				return returnPoint;
			}
		 case ObstacleDecisionZone:
			if(!myKart.ShotComplete && notObs) {
				printf("Move to Obstacle SM\r\n");
				
				ES_Event newEvent = {ToObstacle, 0};
				PostMaster(newEvent);
				
				notObs = false;
				
				return ObstacleEntry;
			}
			else {
				POINT_t returnPoint;
				returnPoint.X = cornerPointMatrix_X[0];
				returnPoint.Y = cornerPointMatrix_Y[0];
				return returnPoint;
			}
			// Set the location via the 4 coners that define a lap
		 case BottomStraight:
			printf("Next Waypoint Is Bottom Right \r\n");
			returnPoint.X = cornerPointMatrix_X[0];
			returnPoint.Y = cornerPointMatrix_Y[0];
			return returnPoint;
		 case RightStraight:
			printf("Next Waypoint Is Top Right \r\n");
			returnPoint.X = cornerPointMatrix_X[1];
			returnPoint.Y = cornerPointMatrix_Y[1];
			return returnPoint;
		 case TopStraight:
			printf("Next Waypoint Is Top Left \r\n");
			returnPoint.X = cornerPointMatrix_X[2];
			returnPoint.Y = cornerPointMatrix_Y[2];
			return returnPoint;
		 case LeftStraight:
			printf("Next Waypoint Is Bottom Left \r\n");
			returnPoint.X = cornerPointMatrix_X[3];
			returnPoint.Y = cornerPointMatrix_Y[3];
			return returnPoint;
		 // The points don't matter for the _in states
		 // Eventually the bot will kick out at 45 degrees
		 case Bin:
			printf("Kick out (Bottom) \r\n");
			returnPoint.X = cornerPointMatrix_X[0];
			returnPoint.Y = cornerPointMatrix_Y[0];
		  exitZone = 1;
			return returnPoint;
		 case Rin:
			printf("Kick out (Right) \r\n");
			returnPoint.X = cornerPointMatrix_X[1];
			returnPoint.Y = cornerPointMatrix_Y[1];
		  exitZone = 2;
			return returnPoint;
		 case Tin:
			printf("Kick out (Top) \r\n");
			returnPoint.X = cornerPointMatrix_X[2];
			returnPoint.Y = cornerPointMatrix_Y[2];
		  exitZone = 3;
			return returnPoint;
		 case Lin:
			printf("Kick out (Left) \r\n");
			returnPoint.X = cornerPointMatrix_X[3];
			returnPoint.Y = cornerPointMatrix_Y[3];
		  exitZone = 4;
			return returnPoint;
		 case DeadZone:
			printf("DeadZone \r\n");
			returnPoint.X = cornerPointMatrix_X[2];
			returnPoint.Y = cornerPointMatrix_Y[2];
			return returnPoint;
	}
	return returnPoint;
}
	
