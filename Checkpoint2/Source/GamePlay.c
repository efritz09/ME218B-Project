/****************************************************************************
 Module
   GamePlay.c

 Revision			Revised by: 
	0.1.1				Alex

 Description
	Gameplay state machine that controls the driving, shooting, and obstacle
    crossing

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
#define ALL_BITS (0xff<<2)

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/
static ES_Event DuringDrivingStateGP(ES_Event Event);
static ES_Event DuringShootingStateGP(ES_Event Event);
static ES_Event DuringObstacleStateGP(ES_Event Event);
static ES_Event DuringPauseStateGP(ES_Event Event);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static GamePlayState CurrentState;
static GamePlayState LastState;

POINT_t nearLinePointShooting = {0,10};
POINT_t nearLinePointObstacle = {10,500};

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
    RunGamePlay

 Parameters
   unsigned char: the event to process

 Returns
   unsigned char: an event to return

 Description
   run function for game play state machine
****************************************************************************/
// make recursive call warning into info
//#pragma MESSAGE INFORMATION C1855
ES_Event RunGamePlay( ES_Event CurrentEvent )
{
   bool MakeTransition = false;/* are we making a state transition? */
   GamePlayState NextState = CurrentState;
   ES_Event EntryEventKind = { ES_ENTRY, 0 };// default to normal entry to new state
   ES_Event ReturnEvent = CurrentEvent; // assume we are not consuming event
   
   //printf("Event posted to gameplay \r\n");
   
   // Query DRS for current position and orientation
   KART_t myKart = QueryMyKart( );

   switch ( CurrentState )
   {
       case DrivingStateGP :
         // Execute During function for driving state
         CurrentEvent = DuringDrivingStateGP(CurrentEvent);
         // Process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT )
         {
            switch (CurrentEvent.EventType)
            {
               case InShootingDecisionZone :
				  printf("In Shooting Decision Zone \r\n");
                  // If shot hasn't been made, move to shooting state
                  if(!myKart.ShotComplete) {
                    NextState = ShootingStateGP;
                    MakeTransition = true;
                  }
                  break;
               case InObstacleDecisionZone :
				   printf("In Obstacle Decision Zone \r\n");
                  // If shot hasn't been made, move to shooting state
                  if(!myKart.ObstacleComplete) {
                    NextState = ObstacleStateGP;
                    MakeTransition = true;
                  }
                  break;
               case CautionFlagDropped :
				  printf("Caution Flag Dropped \r\n");
                  // Move to pause state
                  NextState = PauseStateGP;
                  MakeTransition = true;
                  break;
            }
         }
         break;
       case ShootingStateGP : 
         // Execute During function for shooting state
         CurrentEvent = DuringShootingStateGP(CurrentEvent);
		 if(CurrentEvent.EventType == CautionFlagDropped) {
			printf("Caution Flag Dropped \r\n");
			// Move to pause state
            NextState = PauseStateGP;
            MakeTransition = true;
		 }
		 if(CurrentEvent.EventType == BallFired) {
			printf("Ball Fired \r\n");
			// Move to driving state
            NextState = DrivingStateGP;
			printf("Drive To Point Near Line (Shooting) \r\n");
			// Drive near tape line
			DriveTo(nearLinePointShooting.X, nearLinePointShooting.Y);
            MakeTransition = true;
		 }
         break;
       case ObstacleStateGP :
         // Execute During function for obstacle state
         CurrentEvent = DuringObstacleStateGP(CurrentEvent);
         if(CurrentEvent.EventType == CautionFlagDropped) {
			printf("Caution Flag Dropped \r\n");
			// Move to pause state
            NextState = PauseStateGP;
            MakeTransition = true;
		 }
		 if(CurrentEvent.EventType == ObstacleTraversed) {
			printf("Obstacle Traversed \r\n");
			// Move to driving state
            NextState = DrivingStateGP;
			printf("Drive To Point Near Line (Obstacle) \r\n");
			// Drive near tape line
			DriveTo(nearLinePointObstacle.X, nearLinePointObstacle.Y);
            MakeTransition = true;
		 }
         break;
       case PauseStateGP :
         // Execute During function for pause state
         CurrentEvent = DuringPauseStateGP(CurrentEvent);
         // Process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT )
         {
            switch (CurrentEvent.EventType)
            {
               case FlagDropped :
				  printf("Flag Dropped \r\n");
                  // Move to previous state
                  NextState = LastState;
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
       RunGamePlay(CurrentEvent);
        
       LastState = CurrentState;
       CurrentState = NextState; //Modify state variable

       //   Execute entry function for new state
       CurrentEvent.EventType = ES_ENTRY;
       RunGamePlay(EntryEventKind);
     }
     return(ReturnEvent);
}
/****************************************************************************
 Function
     StartGamePlay

 Parameters
     Event_t CurrentEvent

 Returns
     None

 Description
     Does any required initialization for this state machine
****************************************************************************/
void StartGamePlay ( ES_Event CurrentEvent )
{
   // local variable to get debugger to display the value of CurrentEvent
   volatile ES_Event LocalEvent = CurrentEvent;
   CurrentState = PauseStateGP;
   // call the entry function (if any) for the ENTRY_STATE
   RunGamePlay(CurrentEvent);
}

/****************************************************************************
 Function
     QueryGamePlay

 Parameters
     None

 Returns
     MagControlState The current state of the state machine

 Description
     returns the current state of the state machine
****************************************************************************/
GamePlayState QueryGamePlay ( void )
{
   return(CurrentState);
}

/***************************************************************************
 private functions
 ***************************************************************************/

static ES_Event DuringDrivingStateGP(ES_Event Event)
{
    // Local variable to get debugger to display the value of CurrentEvent
    volatile ES_Event NewEvent = Event;

    // Process ES_ENTRY & ES_EXIT events
    if (Event.EventType == ES_ENTRY) {
		printf("Entered Driving State GP \r\n");
        // Start driving state machine
        StartDriving(Event);
    }
    else if (Event.EventType == ES_EXIT) {
		printf("Exited Driving State GP \r\n");
        // On exit, give the lower levels a chance to clean up first
        NewEvent = RunDriving(Event);
    }
    else {
        // Do the 'during' function for this state
        // Run driving state machine
        NewEvent = RunDriving(Event);
    }
    return(NewEvent);
}

static ES_Event DuringShootingStateGP(ES_Event Event)
{
    // Local variable to get debugger to display the value of CurrentEvent
    volatile ES_Event NewEvent = Event;

    // Process ES_ENTRY & ES_EXIT events
    if (Event.EventType == ES_ENTRY) {
		printf("Entered Shooting State GP \r\n");
        // Start shooting state machine
        StartShooting(Event);
    }
    else if (Event.EventType == ES_EXIT) {
		printf("Exited Shooting State GP \r\n");
        // On exit, give the lower levels a chance to clean up first
        NewEvent = RunShooting(Event);
    }
    else {
        // Do the 'during' function for this state
        // Run shooting state machine
        NewEvent = RunShooting(Event);
    }
    return(NewEvent);
}

static ES_Event DuringObstacleStateGP(ES_Event Event)
{
    // Local variable to get debugger to display the value of CurrentEvent
    volatile ES_Event NewEvent = Event;

    // Process ES_ENTRY & ES_EXIT events
    if (Event.EventType == ES_ENTRY) {
		printf("Entered Obstacle State GP \r\n");
        // Start shooting state machine
        StartObstacle(Event);
    }
    else if (Event.EventType == ES_EXIT) {
		printf("Exited Obstacle State GP \r\n");
        // On exit, give the lower levels a chance to clean up first
        NewEvent = RunObstacle(Event);
    }
    else {
        // Do the 'during' function for this state
        // Run shooting state machine
        NewEvent = RunObstacle(Event);
    }
    return(NewEvent);
}

static ES_Event DuringPauseStateGP(ES_Event Event)
{
    // Local variable to get debugger to display the value of CurrentEvent
    volatile ES_Event NewEvent = Event;

    // Process ES_ENTRY & ES_EXIT events
    if (Event.EventType == ES_ENTRY) {
		printf("Entered Pause State GP \r\n");
		printf("Kill Motors \r\n");
		// Kill motors
        HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
		SetPWMDuty(0,STARBOARD_MOTOR);
		SetPWMDuty(0,PORT_MOTOR);
    }
    else if (Event.EventType == ES_EXIT) {
		printf("Exited Pause State GP \r\n");
        // No exit function
    }
    else {
        // No during functionality
    }
    return(NewEvent);
}

