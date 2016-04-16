/****************************************************************************
 Module
   GamePlay.c

 Revision			Revised by: 
	0.1.1				Alex
	0.1.2       Alex

 Description
	Gameplay state machine that controls the driving, shooting, and obstacle
    crossing

 Edits:
	0.1.1 - Set up as template 
	0.1.2 - Changed to have running game state machine and pause state to remove "hack"
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
static ES_Event DuringRunningGameStateGP(ES_Event Event);
static ES_Event DuringPauseState(ES_Event Event);
static ES_Event DuringWaitForStartState(ES_Event Event);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static GamePlayState CurrentState;

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
   
   // Could querey DRS here for game states if event checkers don't work

   switch ( CurrentState )
   {
       case RunningGameStateGP :
         // Execute During function for driving state
         CurrentEvent = DuringRunningGameStateGP(CurrentEvent);
         // Process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT )
         {
            switch (CurrentEvent.EventType)
            {
               case CautionFlagDropped :
								 printf("Caution Flag Dropped \r\n");
                 // Move to pause state
                 NextState = PauseState;
                 MakeTransition = true;
                 break;
               case GameOver :
								 printf("Game Over \r\n");
                 // Restart to wait for start state
                 NextState = WaitForStartState;
                 MakeTransition = true;
                  break;
               case EmergencyStop :
								 printf("Emergency Stop \r\n");
                 // Move to pause state
                 NextState = PauseState;
                 MakeTransition = true;
                 break;
            }
         }
         break;
				 
       case PauseState :
         // Execute During function for pause state
         CurrentEvent = DuringPauseState(CurrentEvent);
         // Process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT )
         {
            switch (CurrentEvent.EventType)
            {
               case FlagDropped :
							   printf("Caution Over \r\n");
                 // Change entry event to with history
							   EntryEventKind.EventType = ES_ENTRY_HISTORY;
							   // Move to running game state
                 NextState = RunningGameStateGP;
                 MakeTransition = true;
                 break;
							 case GameOver :
								 printf("Game Over \r\n");
                 // Restart to wait for start state
                 NextState = WaitForStartState;
                 MakeTransition = true;
                 break;
            }
         }
         break;
				 
		case WaitForStartState :
         // Execute During function for pause state
         CurrentEvent = DuringWaitForStartState(CurrentEvent);
         // Process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT )
         {
            switch (CurrentEvent.EventType)
            {
               case FlagDropped :
							   printf("Flag Dropped \r\n");
							   // Move to running game state
                 NextState = RunningGameStateGP;
                 MakeTransition = true;
								 
							   // set which Kart we are HERE!
										// read POT and check value 
							 
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
   CurrentState = WaitForStartState;
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

static ES_Event DuringRunningGameStateGP(ES_Event Event)
{
    // Local variable to get debugger to display the value of CurrentEvent
    volatile ES_Event NewEvent = Event;

    // Process ES_ENTRY & ES_EXIT events
    if (Event.EventType == ES_ENTRY || Event.EventType == ES_ENTRY_HISTORY) {
			printf("Entered Running Game State \r\n");
      // Start driving state machine
      StartRunningGame(Event);
    }
    else if (Event.EventType == ES_EXIT) {
			printf("Exited Running Game State \r\n");
      // On exit, give the lower levels a chance to clean up first
      NewEvent = RunRunningGame(Event);
    }
    else {
      // Do the 'during' function for this state
      // Run driving state machine
      NewEvent = RunRunningGame(Event);
    }
    return(NewEvent);
}

static ES_Event DuringPauseState(ES_Event Event)
{
    // Local variable to get debugger to display the value of CurrentEvent
    volatile ES_Event NewEvent = Event;

    // Process ES_ENTRY & ES_EXIT events
    if (Event.EventType == ES_ENTRY) {
			printf("Entered Pause State \r\n");
			printf("Kill Motors \r\n");
			// Kill motors
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
			SetPWMDuty(0,STARBOARD_MOTOR);
			SetPWMDuty(0,PORT_MOTOR);
    }
    else if (Event.EventType == ES_EXIT) {
		printf("Exited Pause State \r\n");
        // No exit function
    }
    else {
        // No during functionality
    }
    return(NewEvent);
}

static ES_Event DuringWaitForStartState(ES_Event Event)
{
    // Local variable to get debugger to display the value of CurrentEvent
    volatile ES_Event NewEvent = Event;

    // Process ES_ENTRY & ES_EXIT events
    if (Event.EventType == ES_ENTRY) {
			printf("Entered Wait For Start State \r\n");
			printf("Kill Motors \r\n");
			// Kill motors
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
			SetPWMDuty(0,STARBOARD_MOTOR);
			SetPWMDuty(0,PORT_MOTOR);
    }
    else if (Event.EventType == ES_EXIT) {
		printf("Exited Wait For Start State \r\n");
        // No exit function
    }
    else {
        // No during functionality
    }
    return(NewEvent);
}
