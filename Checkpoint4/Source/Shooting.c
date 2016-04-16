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
#include "BallShooter.h"
#include <stdio.h>

/*----------------------------- Module Defines ----------------------------*/
// define constants for this machine
#define ALL_BITS (0xff<<2)

#define ONE_SEC 976
#define LOAD_TIME ONE_SEC
#define FIRE_TIME ONE_SEC
#define BEACON_LOW (30000)
#define BEACON_HIGH (33000)

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/
static ES_Event DuringFindBeaconState( ES_Event Event );
static ES_Event DuringFireState( ES_Event Event );
static ES_Event DuringStopState( ES_Event Event );
void BeaconCaptureResponse( void );
bool FindBeacon (void);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static ShootingState CurrentState;
static uint32_t LastCapture;
static uint32_t BeaconPeriod;
static bool BeaconLeftTurn = false;
//static bool ValidLastPeriod;
static bool Beacon = false; //do we search for beacon? T or F

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
               case DetectedBeacon: //If event is event one
								printf("Detected Beacon \r\n");
							 
								// Murder the motors
								HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
								SetPWMDuty(0,STARBOARD_MOTOR);
								SetPWMDuty(0,PORT_MOTOR);
							 
								NextState = FireState;//Decide what the next state will be
								MakeTransition = true; //mark that we are taking a transition
								break;
							 case ES_TIMEOUT:
								 if (CurrentEvent.EventParam == CHECK_TIMER) {
									 printf("missed it, FUCK!\r\n");
									 if(BeaconLeftTurn) {
										 		// Set left motor to drive in reverse
												HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~BIT3HI;
												// Set right motor to drive forward
												HWREG(GPIO_PORTB_BASE + ALL_BITS) |= BIT2HI;
									 }else {
										 // Set right motor to drive in reverse
												HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~BIT2HI;
												// Set left motor to drive forward
												HWREG(GPIO_PORTB_BASE + ALL_BITS) |= BIT3HI;
									 }
									 ES_Timer_InitTimer(CHECK_TIMER,ONE_SEC*2);
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
									NextState = StopState;//Decide what the next state will be
									MakeTransition = true; //mark that we are taking a transition
								}
                break;
            }
         }
         break;
				 case StopState :
         // Execute during function
         CurrentEvent = DuringStopState(CurrentEvent);
					 if(CurrentEvent.EventType == InShootingDecisionZone){
							FireBallShooter();
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
			
			printf("Rotate Slowly \r\n");
			// Counter-clockwise rotation
			// Set left motor to drive in reverse
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~BIT3HI;
			// Set right motor to drive forward
			HWREG(GPIO_PORTB_BASE + ALL_BITS) |= BIT2HI;
			// Set PWM to 1/4 speed
			SetPWMDuty(HALF_SPEED_PORT/2,PORT_MOTOR);
			SetPWMDuty(HALF_SPEED_STARBOARD/2,STARBOARD_MOTOR);
			
			ES_Timer_InitTimer(CHECK_TIMER,ONE_SEC);
			
			printf("Load Ball \r\n");
			// Get it in
			// Open gate to let one ball out
			printf("Open Gate (let one ball out) \r\n");
			ES_Timer_InitTimer(LOAD_BALL_TIMER,LOAD_TIME);
			
			Beacon = true;
    }
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Find Beacon State \r\n");
        // No exit functionality 
		Beacon = false;
    }
	else {
			// No during functionallity
    }
    return( Event );  // Don't remap event
}

static ES_Event DuringFireState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY) {
		printf("Entered Fire State \r\n");
		printf("Start Fire Timer \r\n");
		// Start load ball timer
		ES_Timer_InitTimer(FIRE_BALL_TIMER,FIRE_TIME);
		// Activate twanger
		printf("Activate Twanger \r\n");
		FireBallShooter();
		
    }
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Fire State \r\n");
        // No exit functionality
		
				// Reset twanger
		
    }
	else {
		// No during functionality
    }
    return( Event );  // Don't remap event
}

void BeaconCaptureResponse( void )
{
	uint32_t ThisCapture;
	
	// Clear the interrupt source
	HWREG(WTIMER0_BASE+TIMER_O_ICR) = TIMER_ICR_CAECINT;
	
	// Read the capture value and calculate the period
	ThisCapture = HWREG(WTIMER0_BASE+TIMER_O_TAR);
	BeaconPeriod = (ThisCapture - LastCapture);
	//printf("%d \r\n", BeaconPeriod);
	
	if(Beacon) {
		if( BeaconPeriod > BEACON_LOW && BeaconPeriod < BEACON_HIGH && Beacon ){
			ES_Event newEvent = {DetectedBeacon};
			PostMaster(newEvent);
			printf("Beacon Located\n\r");
		}
	}
//	if(BeaconPeriod > BEACON_LOW/2 && BeaconPeriod < BEACON_HIGH*2 )
//	{
//		ValidLastPeriod = true;
//	}
//	else ValidLastPeriod = false;
	
	// Update LastCapture to ThisCapture
	LastCapture = ThisCapture;
}
static ES_Event DuringStopState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY) {
			printf("Murder Motors\r\n");
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
			SetPWMDuty(0,STARBOARD_MOTOR);
			SetPWMDuty(0,PORT_MOTOR);
		
    }
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Fire State \r\n");
        // No exit functionality
		
				// Reset twanger
		
    }
	else {
		// No during functionality
    }
    return( Event );  // Don't remap event
}
