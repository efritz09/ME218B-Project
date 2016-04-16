/****************************************************************************
 Module
   Obstacle.c

 Revision			Revised by: 
	0.1.1				Alex

 Description
	Obstacle crossing state machine that controls traversing the obstacle
	
	could add swinging motion when finding line to make sure we get there

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

#define OBSTACLE_ORIENTATION 90

#define SLIGHT_RIGHT_PORT HALF_SPEED_PORT - 5
#define SLIGHT_RIGHT_STARBOARD HALF_SPEED_PORT + 5

#define SLIGHT_LEFT_PORT HALF_SPEED_PORT + 5
#define SLIGHT_LEFT_STARBOARD HALF_SPEED_PORT - 5


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/
static ES_Event DuringTapeFindingState( ES_Event Event);
static ES_Event DuringOnLineState( ES_Event Event);
static ES_Event DuringOffLineState( ES_Event Event);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static ObstacleState CurrentState;
static bool arrived = false;
static bool oriented = false;

POINT_t obstaclePoint = {10,100};

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
	
   //printf("Event posted to obstacle \r\n");
	
   switch ( CurrentState )
   {
       case TapeFindingState :
         // Execute during function
         CurrentEvent = DuringTapeFindingState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case AtNextPoint:
				  arrived = true;
				  printf("At Obstacle Point (set arrived flag to true) \r\n");
			      printf("Orient \r\n");
                  TurnTheta(OBSTACLE_ORIENTATION);
                  break;
			   case AtNextAngle:
				   if(arrived) {
					  // Go straight
					  oriented = true;
					  printf("At Correct Orinetation (set orirented flag to true) \r\n");
					   printf("Drive Forward (1/4 Speed) \r\n");
					  // Set left motor to drive forward
					  HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT3HI;
					  // Set right motor to drive forward
					  HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
					  // Set PWM to (Quarter Speed)
					  SetPWMDuty(FULL_SPEED_PORT/4,PORT_MOTOR);
					  SetPWMDuty(FULL_SPEED_STARBOARD/4,STARBOARD_MOTOR);
				   }
                  break;
			   case MiddleTapeTripped:
				  if(arrived && oriented) {
					  printf("Middle Tape Sensor Tripped \r\n");
					  printf("Drive Forward (1/2 Speed) \r\n");
					  // Go straight
					  // Set left motor to drive forward
					  HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT3HI;
					  // Set right motor to drive forward
					  HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
					  // Set PWM to (Half Speed)
					  SetPWMDuty(FULL_SPEED_PORT/2,PORT_MOTOR);
					  SetPWMDuty(FULL_SPEED_STARBOARD/2,STARBOARD_MOTOR);
				   
					  // Transition into on line state
					  NextState = OnLineState;//Decide what the next state will be
					  MakeTransition = true; //mark that we are taking a transition
				  }
                  break;
            }
         }
         break;
        case OnLineState :
         // Execute during function
         CurrentEvent = DuringOnLineState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case MiddleTapeLost: //If event is event one
				  printf("Middle Tape Lost \r\n");
                  NextState = OffLineState;//Decide what the next state will be
                  MakeTransition = true; //mark that we are taking a transition
                  break;
            }
         }
         break;
		case OffLineState :
         // Execute during function
         CurrentEvent = DuringOffLineState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case MiddleTapeTripped:
				  printf("Middle Tape Sensor Tripped \r\n");
			      printf("Drive Forward (1/2 Speed) \r\n");
				  // Go straight
				  // Set left motor to drive forward
			      HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT3HI;
				  // Set right motor to drive forward
				  HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
				  // Set PWM to (Half Speed)
				  SetPWMDuty(FULL_SPEED_PORT/2,PORT_MOTOR);
				  SetPWMDuty(FULL_SPEED_STARBOARD/2,STARBOARD_MOTOR);
	
                  // Transition into on line state
			      NextState = OnLineState;//Decide what the next state will be
                  MakeTransition = true; //mark that we are taking a transition
                  break;
			   case LeftTapeTripped:
				  printf("Left Tape Sensor Tripped \r\n");
			      printf("Veer Right \r\n");
                  // Slight right
				  // Set left motor to drive forward
			      HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT3HI;
				  // Set right motor to drive forward
				  HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
				  // Set PWM to make a slight right
				  SetPWMDuty(SLIGHT_RIGHT_PORT,PORT_MOTOR);
				  SetPWMDuty(SLIGHT_RIGHT_STARBOARD,STARBOARD_MOTOR);
                  break;
			   case RightTapeTripped:
				  printf("Right Tape Sensor Tripped \r\n");
			      printf("Veer Left \r\n");
                  // Slight left
				  // Set left motor to drive forward
			      HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT3HI;
				  // Set right motor to drive forward
				  HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
				  // Set PWM to make a slight left
				  SetPWMDuty(SLIGHT_LEFT_PORT,PORT_MOTOR);
				  SetPWMDuty(SLIGHT_LEFT_STARBOARD,STARBOARD_MOTOR);
                  break;
            }
         }
    }
    //   If we are making a state transition
    if (MakeTransition == true)
    {
       //   Execute exit function for current state
       CurrentEvent.EventType = ES_EXIT;
       RunObstacle(CurrentEvent);

       CurrentState = NextState; //Modify state variable

       //   Execute entry function for new state
       CurrentEvent.EventType = ES_ENTRY;
       RunObstacle(CurrentEvent);
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
	ES_Event LocalEvent = CurrentEvent;
	CurrentState = TapeFindingState;
	
	arrived = false;
	oriented = false;
	
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

static ES_Event DuringTapeFindingState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY) {
		
		printf("Entered Tape Finding State \r\n");
		
        // Drive into obstacle zone
		DriveTo(obstaclePoint.X, obstaclePoint.Y);
		printf("Drive to Obstacle Point \r\n");
    }
	else if ( Event.EventType == ES_EXIT) {
        // No exit functionality 
		printf("Exited Tape Finding State \r\n");
    }
	else {
		// No during functionallity
    }
    return( Event );  // Don't remap event
}

static ES_Event DuringOnLineState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY) {
        // No entry functionality 
		printf("Entered On Line State \r\n");
    }
	else if ( Event.EventType == ES_EXIT) {
        // No exit functionality 
		printf("Exited On Line State \r\n");
    }
	else {
		// No during functionality
    }
    return( Event );  // Don't remap event
}

static ES_Event DuringOffLineState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY) {
        // No entry functionality 
		printf("Entered Off Line State \r\n");
    }
	else if ( Event.EventType == ES_EXIT) {
        // No exit functionality 
		printf("Exited Off Line State \r\n");
    }
	else {
		// No during functionality
    }
    return( Event );  // Don't remap event
}

