/****************************************************************************
 Module
   BallShooter.c

 Revision			Revised by: 
	0.1.1					Eric			2/25/2015

 Description
  State Machine for launching and reloading the balls
	 
 Edits:

 Notes:
	Ball Servo Limits: Width = 500-2500 (moves ~ 180 degrees)
	Hopper Servo Limits: Not Tested
	
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

// ES_framework headers
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Port.h"
#include "ES_Types.h"
#include "ES_Events.h"

// TIVA headers
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_pwm.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "bitdefs.h"

// Module headers
#include "PWM.h"
#include "BallShooter.h"

/*----------------------------- Module Defines ----------------------------*/
// Symbolic defines for time
#define ONE_SEC 							976				// In ms (This isnt really 1 sec right?)
#define BALL_TIME ONE_SEC

// Symbolic defines for servo positions/PWM channel
#define SHOOTER_SET						1400
#define SHOOTER_FLICK					500	
#define SHOOTER_RESET					2000	
#define SHOOTER_CHANNEL				2
#define HOPPER_SET						1000		// place holder width for now
#define HOPPER_RELEASE				1500		// place holder width for now
#define HOPPER_CHANNEL				3



/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behaviour of this service
*/

void FlickerSetStartPosition( void );
void FlickerFlickBall( void );
void FlickerReset( void );
void HopperSet( void );
void HopperRelease( void );

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static ShooterState BallShootState;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
	InitBallDestroyer

 Parameters
	none

 Returns
	none

 Description
	Initializes everything for the shooter
****************************************************************************/
bool InitBallShooter(void)
{
	FlickerReset(); 
	HopperSet();
	
	BallShootState = ShooterResetting;
	ES_Timer_InitTimer(BALL_SHOOTER_TIMER,BALL_TIME);
	printf("Ball Shooter Initialized\r\n");
	return true;
}

void FireBallShooter(void)
{
	FlickerFlickBall();
	ES_Timer_InitTimer(BALL_SHOOTER_TIMER,BALL_TIME);
}


/****************************************************************************
 Function
	RunBallShooter

 Description
  Runs a pseudo-state machine to test our servo motors
****************************************************************************/
ES_Event RunBallShooter ( ES_Event ThisEvent )
{
	ES_Event ReturnEvent = {ES_NO_EVENT};
	
	if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == BALL_SHOOTER_TIMER){
		switch (BallShootState)
			{
			case ShooterReady:
				//the thing has been shot, time to reset it
				FlickerReset();
				//HopperSet();
				BallShootState = ShooterResetting;
				ES_Timer_InitTimer(BALL_SHOOTER_TIMER,BALL_TIME);
				break;
			case ShooterResetting:
				//set the start position and reload the ball
				FlickerSetStartPosition();
				//HopperRelease();
				//ES_Timer_InitTimer(BALL_SHOOTER_TIMER,ONE_SEC);
				BallShootState = ShooterReady;
				break;
		}
	}
		
		
		
	/*
	// Test harness for shooter
	if( ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == SHOOTER_TIMER)
	{
		//static int ShooterState = 1;
		if( ShooterState == 1)
		{
			FlickerSetStartPosition();
			ShooterState = 2;
			printf("Twang Machine: StartPosition \n\r");
		}
		else if(ShooterState ==2 )
		{
			FlickerFlickBall();
			ShooterState = 3;
			printf("Twang Machine: FlickBall\n\r");
			
		}
		else if(ShooterState ==3 )
		{
			FlickerReset();
			ShooterState = 1;
			printf("Twang Machine: Reset \n\r");
		}
		
		ES_Timer_InitTimer(SHOOTER_TIMER, 3000);
	}
	
	// Test harness for hopper
	if( ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == HOPPER_TIMER)
	{
		static int HopperState = 1;
		if( HopperState == 1)
		{
			HopperSet();
			HopperState = 2;
			printf("Hopper: Ball Release \n\r");
			ES_Timer_InitTimer(HOPPER_TIMER, 5500);
		}
		else if(HopperState ==2 )
		{
			HopperRelease();
			HopperState = 1;
			printf("Hopper: Ball Load\n\r");
			ES_Timer_InitTimer(HOPPER_TIMER, 3500);
			
		}
		
		
	}
	*/
	return ReturnEvent;
}

/****************************************************************************
 Function
	PostBallShooter

 Description
    Posts an event to this service's queue
****************************************************************************/
bool PostBallShooter ( ES_Event ThisEvent )
{
	
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
	SetStartPosition

 Description
    
****************************************************************************/
void FlickerSetStartPosition( void )
{
	SetPWMWidth( SHOOTER_SET, SHOOTER_CHANNEL);
	printf("Flicker is set to Start position\r\n");
	HopperRelease();
}

/****************************************************************************
 Function
	FlickBall

 Description
   
****************************************************************************/
void FlickerFlickBall( void )
{
	SetPWMWidth( SHOOTER_FLICK, SHOOTER_CHANNEL);
	printf("Flicker has fired the ball\r\n");
}

/****************************************************************************
 Function
	Reset

 Description
    
****************************************************************************/
void FlickerReset( void )
{
	SetPWMWidth( SHOOTER_RESET, SHOOTER_CHANNEL);
	printf("Flicker has been reset\r\n");
	HopperSet();
}

/****************************************************************************
 Function
	HopperSet

 Description
   
****************************************************************************/
void HopperSet( void )
{
	SetPWMWidth( HOPPER_SET, HOPPER_CHANNEL);
	printf("hopper is set\r\n");
}

/****************************************************************************
 Function
	HopperRelease

 Description
    
****************************************************************************/
void HopperRelease( void )
{
	SetPWMWidth( HOPPER_RELEASE, HOPPER_CHANNEL);
	printf("hopper is released\r\n");
}
