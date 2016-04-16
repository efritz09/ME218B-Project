/****************************************************************************
 Module
   RunningGame.c

 Revision			Revised by: 
	0.1.1				Alex

 Description
	RunningGame state machine that controls the driving, shooting, and obstacle
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
#include "RunningGame.h"
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
static ES_Event DuringDrivingStateSM(ES_Event Event);
static ES_Event DuringShootingStateSM(ES_Event Event);
static ES_Event DuringObstacleStateSM(ES_Event Event);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static RunningGameState CurrentState;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
    RunRunningGame

 Parameters
   unsigned char: the event to process

 Returns
   unsigned char: an event to return

 Description
   run function for game play state machine
****************************************************************************/
// make recursive call warning into info
//#pragma MESSAGE INFORMATION C1855
ES_Event RunRunningGame( ES_Event CurrentEvent )
{
   bool MakeTransition = false;/* are we making a state transition? */
   RunningGameState NextState = CurrentState;
   ES_Event EntryEventKind = { ES_ENTRY, 0 };// default to normal entry to new state
   ES_Event ReturnEvent = CurrentEvent; // assume we are not consuming event
   
   //printf("Event posted to RunningGame \r\n");
   
   // Query DRS for current position and orientation
   KART_t myKart = QueryMyKart( );

   switch ( CurrentState )
   {
		 if(CurrentEvent.EventType == ToObstacle) {
				printf("To Obstacle \r\n");
				// Move to obstacle state
				NextState = ObstacleStateSM;
				MakeTransition = true;
		 }
         break;
       case ShootingStateSM : 
         // Execute During function for shooting state
         CurrentEvent = DuringShootingStateSM(CurrentEvent);
				 if(CurrentEvent.EventType == CautionFlagDropped) {
						printf("murder motors\r\n");
						// Murder the motors
						HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
						SetPWMDuty(0,STARBOARD_MOTOR);
						SetPWMDuty(0,PORT_MOTOR);
					 printf("To Obstacle \r\n");
						// Move to driving state
						NextState = ObstacleStateSM;
						MakeTransition = true;
					 
						
					 
					 
					 
					 
				 }
			break;
       case ObstacleStateSM :
         // Execute During function for obstacle state
         CurrentEvent = DuringObstacleStateSM(CurrentEvent);
         break;
    }
   
    //   If we are making a state transition
    if (MakeTransition == true)
    {
       //   Execute exit function for current state
       CurrentEvent.EventType = ES_EXIT;
       RunRunningGame(CurrentEvent);
        
       CurrentState = NextState; //Modify state variable

       //   Execute entry function for new state
       CurrentEvent.EventType = ES_ENTRY;
       RunRunningGame(EntryEventKind);
     }
     return(ReturnEvent);
}
/****************************************************************************
 Function
     StartRunningGame

 Parameters
     Event_t CurrentEvent

 Returns
     None

 Description
     Does any required initialization for this state machine
****************************************************************************/
void StartRunningGame ( ES_Event CurrentEvent )
{
   // local variable to get debugger to display the value of CurrentEvent
   volatile ES_Event LocalEvent = CurrentEvent;
	printf("START RG \r\n");
   CurrentState = ShootingStateSM;
   // call the entry function (if any) for the ENTRY_STATE
   RunRunningGame(CurrentEvent);
}

/****************************************************************************
 Function
     QueryRunningGame

 Parameters
     None

 Returns
     MagControlState The current state of the state machine

 Description
     returns the current state of the state machine
****************************************************************************/
RunningGameState QueryRunningGame ( void )
{
   return(CurrentState);
}

/***************************************************************************
 private functions
 ***************************************************************************/

static ES_Event DuringShootingStateSM(ES_Event Event)
{
    // Local variable to get debugger to display the value of CurrentEvent
    volatile ES_Event NewEvent = Event;

    // Process ES_ENTRY & ES_EXIT events
    if (Event.EventType == ES_ENTRY) {
			printf("Entered Shooting State SM \r\n");
      // Start shooting state machine
      StartShooting(Event);
    }
    else if (Event.EventType == ES_EXIT) {
			printf("Exited Shooting State SM \r\n");
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

static ES_Event DuringObstacleStateSM(ES_Event Event)
{
    // Local variable to get debugger to display the value of CurrentEvent
    volatile ES_Event NewEvent = Event;

    // Process ES_ENTRY & ES_EXIT events
    if (Event.EventType == ES_ENTRY) {
			printf("Entered Obstacle State SM \r\n");
      // Start shooting state machine
      StartObstacle(Event);
    }
    else if (Event.EventType == ES_EXIT) {
		  printf("Exited Obstacle State SM \r\n");
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

