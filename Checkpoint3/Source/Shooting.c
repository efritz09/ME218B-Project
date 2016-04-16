/****************************************************************************
 Module
   Shooting.c

 Revision			Revised by: 
	0.1.1				Alex

 Description
	Driving state machine that controls the shooting

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
#define ALL_BITS (0xff<<2)

#define ONE_SEC 976
#define LOAD_TIME ONE_SEC
#define FIRE_TIME ONE_SEC


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/
static ES_Event DuringFindBeaconState( ES_Event Event);
static ES_Event DuringFoundBeaconRightState( ES_Event Event);
static ES_Event DuringFoundBeaconLeftState( ES_Event Event);
static ES_Event DuringLoadingBallState( ES_Event Event);
static ES_Event DuringFireState( ES_Event Event);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static ShootingState CurrentState;
static bool arrived = false;

POINT_t shootingPoint = {10,10};

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
    RunShooting

 Parameters
   unsigned char: the event to process

 Returns
   unsigned char: an event to return

 Description
   implements shooting state machine using sensor data
****************************************************************************/
ES_Event RunShooting( ES_Event CurrentEvent )
{
   bool MakeTransition = false;/* are we making a state transition? */
   ShootingState NextState = CurrentState;
   ES_Event ReturnEvent = CurrentEvent; // assume we are not consuming event
	
   //printf("Event posted to shooting \r\n");

   switch ( CurrentState )
   {
       case FindBeaconState :
         // Execute during function
         CurrentEvent = DuringFindBeaconState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case DetectedLeftBeacon: //If event is event one
				  printf("Detected Left Beacon \r\n");
				  if(arrived) {
					NextState = FoundBeaconLeftState;//Decide what the next state will be
					MakeTransition = true; //mark that we are taking a transition
				  }
                  break;
			   case DetectedRightBeacon: //If event is event one
				  printf("Detected Left Beacon \r\n");
				  if(arrived) {
					NextState = FoundBeaconRightState;//Decide what the next state will be
					MakeTransition = true; //mark that we are taking a transition
				  }
                  break;
			   case AtNextPoint:
				  printf("At Shooting Point (set arrived flag true) \r\n");
				  // Begin rotating clockwise direction
			   	  arrived = true;
			      printf("Rotate Clockwise \r\n");
				  // Set left motor to drive forward
				  HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT3HI);
				  // Set right motor to drive in reverse
				  HWREG(GPIO_PORTB_BASE + ALL_BITS) |= BIT2HI;
				  // Set PWM to (50%)
				  SetPWMDuty(HALF_SPEED_PORT,PORT_MOTOR);
				  SetPWMDuty(100 - HALF_SPEED_STARBOARD,STARBOARD_MOTOR);
			      break;
            }
         }
         break;
        case FoundBeaconLeftState :
         // Execute during function
         CurrentEvent = DuringFoundBeaconLeftState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case DetectedRightBeacon: //If event is event one
				  printf("Detected Right Beacon \r\n");
                  NextState = LoadingBallState;//Decide what the next state will be
                  MakeTransition = true; //mark that we are taking a transition
                  break;
            }
         }
         break;
		case FoundBeaconRightState :
         // Execute during function
         CurrentEvent = DuringFoundBeaconRightState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case DetectedLeftBeacon: //If event is event one
				  printf("Detected Left Beacon \r\n");
                  NextState = LoadingBallState;//Decide what the next state will be
                  MakeTransition = true; //mark that we are taking a transition
                  break;
            }
         }
         break;
		case LoadingBallState :
         // Execute during function
         CurrentEvent = DuringLoadingBallState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case ES_TIMEOUT : //If event is event one
				  if(CurrentEvent.EventParam == LOAD_BALL_TIMER) {
					  printf("Ball Loaded \r\n");
					  NextState = FireState;//Decide what the next state will be
					  MakeTransition = true; //mark that we are taking a transition
				  }
                break;
            }
         }
         break;
	    case FireState :
         // Execute during function
         CurrentEvent = DuringFireState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case ES_TIMEOUT : //If event is event one
				  if(CurrentEvent.EventParam == FIRE_BALL_TIMER) {
					  printf("Ball Fired \r\n");
					  ES_Event newEvent = {BallFired, 0};
					  PostMaster(newEvent);
				  }
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
       RunShooting(CurrentEvent);

       CurrentState = NextState; //Modify state variable

       //   Execute entry function for new state
       CurrentEvent.EventType = ES_ENTRY;
       RunShooting(CurrentEvent);
     }
     return(ReturnEvent);
}
/****************************************************************************
 Function
     StartShooting

 Parameters
     Event_t CurrentEvent

 Returns
     None

 Description
     Starts in find beacon state
****************************************************************************/
void StartShooting ( ES_Event CurrentEvent )
{
	// local variable to get debugger to display the value of CurrentEvent
	//ES_Event LocalEvent = CurrentEvent;
	arrived = false;
	
	CurrentState = FindBeaconState;

   // Call the entry function (if any) for the ENTRY_STATE
   RunShooting(CurrentEvent);
}

/****************************************************************************
 Function
     QueryShooting

 Parameters
     None

 Returns
     unsigned char The current state of the Template state machine

 Description
     returns the current state of the shooting state machine
****************************************************************************/
ShootingState QueryShooting ( void )
{
   return(CurrentState);
}

/***************************************************************************
 private functions
 ***************************************************************************/

static ES_Event DuringFindBeaconState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY) {
		printf("Entered Find Beacon State \r\n");
        // Drive into shooting zone
		printf("Drive To Shooting Point \r\n");
		DriveTo(shootingPoint.X, shootingPoint.Y);
    }
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Find Beacon State \r\n");
        // No exit functionality 
    }
	else {
		// No during functionallity
    }
    return( Event );  // Don't remap event
}

static ES_Event DuringFoundBeaconRightState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY) {
		printf("Entered Found Right Beacon State \r\n");
		printf("Turn Counter-Clockwise\r\n");
        // Turn counter-clockwise
		// Set left motor to drive in reverse
		HWREG(GPIO_PORTB_BASE + ALL_BITS) |= BIT3HI;
		// Set right motor to drive forward
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI);
		// Set PWM to (50%)
		SetPWMDuty(100 - HALF_SPEED_PORT,PORT_MOTOR);
		SetPWMDuty(HALF_SPEED_STARBOARD,STARBOARD_MOTOR);
    }
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Found Right Beacon State \r\n");
        // No exit functionality 
    }
	else {
		// No during functionality
    }
    return( Event );  // Don't remap event
}

static ES_Event DuringFoundBeaconLeftState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY) {
		printf("Entered Found Left Beacon State \r\n");
		printf("Turn Clockwise\r\n");
        // Turn clockwise
		// Set left motor to drive forward
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT3HI);
		// Set right motor to drive in reverse
		HWREG(GPIO_PORTB_BASE + ALL_BITS) |= BIT2HI;
		// Set PWM to (50%)
		SetPWMDuty(HALF_SPEED_PORT,PORT_MOTOR);
		SetPWMDuty(100 - HALF_SPEED_STARBOARD,STARBOARD_MOTOR);
    }
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Found Left Beacon State \r\n");
        // No exit functionality 
    }
	else {
		// No during functionality
    }
    return( Event );  // Don't remap event
}

static ES_Event DuringLoadingBallState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY) {
		printf("Entered Loading Ball State \r\n");
		printf("Start Timer \r\n");
		// Start load ball timer
		ES_Timer_InitTimer(LOAD_BALL_TIMER,LOAD_TIME);
		// Open gate to let one ball out
		printf("Open Gate (let one ball out) \r\n");
		
    }
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Load Ball State \r\n");
        // No exit functionality 
    }
	else {
		// No during functionality
    }
    return( Event );  // Don't remap event
}

static ES_Event DuringFireState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY) {
		printf("Entered Fire State \r\n");
		printf("Start Timer \r\n");
		// Start load ball timer
		ES_Timer_InitTimer(FIRE_BALL_TIMER,FIRE_TIME);
		// Activate twanger
		printf("Activate Twanger \r\n");
		
    }
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Fire State \r\n");
        // No exit functionality
    }
	else {
		// No during functionality
    }
    return( Event );  // Don't remap event
}

