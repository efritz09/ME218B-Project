/****************************************************************************
 Module
   Obstacle.c

 Revision			Revised by: 
	0.1.1				Alex
	0.2.1				Alex

 Description
	Obstacle crossing state machine that controls traversing the obstacle
	
	could add swinging motion when finding line to make sure we get there

 Edits:
	0.1.1 - Set up as template 
	0.2.1 - Update obstacle state machine for tape finding with two analog tape sensors
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

#define OBSTACLE_ORIENTATION 90

#define SLIGHT_RIGHT_PORT HALF_SPEED_PORT - 5
#define SLIGHT_RIGHT_STARBOARD HALF_SPEED_STARBOARD + 5

#define SLIGHT_LEFT_PORT HALF_SPEED_PORT + 5
#define SLIGHT_LEFT_STARBOARD HALF_SPEED_STARBOARD - 5

#define smallThetaPWM 25
#define TapeFindTime 100
#define OffTapeTime 100


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/
static ES_Event DuringFullSpeed( ES_Event Event); 
static ES_Event DuringQuarterSpeed( ES_Event Event);
static ES_Event DuringStop( ES_Event Event);


/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static ObstacleState CurrentState;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
    RunObstacle

 Parameters
   unsigned char: the event to process

 Returns
   unsigned char: an event to return

 Description
   implements obstacle state machine using DRS data and tape sensor data
****************************************************************************/
ES_Event RunObstacle( ES_Event CurrentEvent )
{
   bool MakeTransition = false;/* are we making a state transition? */
   ObstacleState NextState = CurrentState;
   ES_Event ReturnEvent = CurrentEvent; // assume we are not consuming event
	 ES_Event EntryEventKind = { ES_ENTRY, 0 };// default to normal entry to new state
   //printf("Event posted to obstacle \r\n");
	
   switch ( CurrentState )
   {
		 case FullSpeed:
			 DuringFullSpeed(CurrentEvent);
			 if(CurrentEvent.EventType == ES_TIMEOUT && CurrentEvent.EventParam == FS_TIMER){
				 NextState = QuarterSpeed;
				 printf("Transition to Quarter Speed\r\n");
				 MakeTransition = true;
			 }
			break;
		 case QuarterSpeed:
			 DuringQuarterSpeed(CurrentEvent);
			 if(CurrentEvent.EventType == ES_TIMEOUT && CurrentEvent.EventParam == QS_TIMER){
				 NextState = Stop;
				 printf("stoping the obstacle\r\n");
				 MakeTransition = true;
			 }
			break;
		 case Stop:
			 DuringStop(CurrentEvent);
			 if(CurrentEvent.EventType == CautionFlagDropped){
				 NextState = FullSpeed;
				 MakeTransition = true;
			 }
			break;
	 }
	 
	 if (MakeTransition == true)
    {
       //   Execute exit function for current state
       CurrentEvent.EventType = ES_EXIT;
       RunObstacle(CurrentEvent);
        
       CurrentState = NextState; //Modify state variable

       //   Execute entry function for new state
       CurrentEvent.EventType = ES_ENTRY;
       RunObstacle(EntryEventKind);
     }
     return(ReturnEvent);

 }
	    

/****************************************************************************
 Function
     StartObstacle

 Parameters
     Event_t CurrentEvent

 Returns
     None

 Description
     Starts in at tape finding state
****************************************************************************/
void StartObstacle ( ES_Event CurrentEvent )
{
	// local variable to get debugger to display the value of CurrentEvent
	//ES_Event LocalEvent = CurrentEvent;
	CurrentState = Stop;
	printf("starting obstacle\r\n");
   // Call the entry function (if any) for the ENTRY_STATE
   RunObstacle(CurrentEvent);
}

/****************************************************************************
 Function
     QueryObstacle

 Parameters
     None

 Returns
     unsigned char The current state of the Template state machine

 Description
     returns the current state of the obstacle state machine
****************************************************************************/
ObstacleState QueryObstacle ( void )
{
   return(CurrentState);
}

/***************************************************************************
 private functions
 ***************************************************************************/
static ES_Event DuringFullSpeed( ES_Event Event )
{
	
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY) {
		printf("Entered Full Speed State \r\n");
		ES_Timer_InitTimer(FS_TIMER,ONE_SEC*1.2);
    // Rotate to orient toward the obstacle
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
		SetPWMDuty(99,STARBOARD_MOTOR);
		SetPWMDuty(99,PORT_MOTOR);
		printf("Start Full Speed \r\n");
	}
	else if ( Event.EventType == ES_EXIT) {
        // No exit functionality 
		printf("Exited Full Speed State \r\n");
	}
	else {
		// No during functionallity
	}
    return( Event );  // Don't remap event
}

static ES_Event DuringQuarterSpeed( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY) {
		printf("Entered Quarter Speed State \r\n");
		ES_Timer_InitTimer(QS_TIMER,ONE_SEC*2);
    // Rotate to orient toward the obstacle
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
		SetPWMDuty(HALF_SPEED_STARBOARD,STARBOARD_MOTOR);
		SetPWMDuty(HALF_SPEED_PORT,PORT_MOTOR);
		printf("Start Quarter Speed \r\n");
	}
	else if ( Event.EventType == ES_EXIT) {
    // No exit functionality 
		printf("Exited Quarter Speed State \r\n");
	}
	else {
		// No during functionallity
    }
    return( Event );  // Don't remap event
}

static ES_Event DuringStop( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY) {
			printf("Murder Motors\r\n");
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
			SetPWMDuty(0,STARBOARD_MOTOR);
			SetPWMDuty(0,PORT_MOTOR);
    }
	else if ( Event.EventType == ES_EXIT) {
    // No exit functionality 
		printf("Exited Stop State \r\n");
    }
	else {
		// No during functionality
    }
    return( Event );  // Don't remap event
}
