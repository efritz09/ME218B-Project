/****************************************************************************
 Module
	Drive.c

 Revision			Revised by: 
	0.1.1				Alex

 Description
	Drive module initializes PWM and motor pins and provides public functions useful
	for driving the machine

 Edits:
	0.1.1 - Set up as template to outline functionality of driving functions
				
			potential problem with accuracy of turn theta function as it is
			called from another function and not error checked there
****************************************************************************/
// If we are debugging and setting our own Game/KART states
#define TEST

/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include <stdint.h>
#include <stdbool.h>
#include <cmath>

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Port.h"
#include "ES_Types.h"
#include "ES_Events.h"

#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_pwm.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "inc/hw_ssi.h"
#include "inc/hw_nvic.h"
#include "driverlib/ssi.h"
#include "bitdefs.h"

#include "Drive.h"
#include "SPITemplate.h"
#include "PWM.h"
#include "Master.h"



/*----------------------------- Module Defines ----------------------------*/
#define BitsPerNibble  4
#define TicksPerMS 40000
#define ALL_BITS (0xff<<2)
#define PI 3.14159265

#define PosResolution 100
#define AngleResolution 100


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static uint16_t lastX;
static uint16_t lastY;
static uint16_t lastTheta;
static bool fromDriveTo = false;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
	InitDrive

 Parameters
	uint8_t : the priorty of this service

 Returns
	bool, false if error in initialization, true otherwise

 Description
	Intializes PWM pins and other motor pins
****************************************************************************/
bool InitDrive ( uint8_t Priority )
{
	ES_Event ThisEvent;
	MyPriority = Priority;

	printf("Drive Initialized\n\r");
	
	// Post the initial transition event
	ThisEvent.EventType = ES_INIT;
	if (ES_PostToService( MyPriority, ThisEvent) == true)
	{
		return true;
	}else
	{
      return false;
	}
}

/****************************************************************************
 Function
	PostDrive

 Parameters
    ES_Event ThisEvent ,the event to post to the queue

 Returns
    bool false if the Enqueue operation failed, true otherwise

 Description
    Posts an event to this service's queue
****************************************************************************/
bool PostDrive ( ES_Event ThisEvent )
{
	
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
	RunDrive

 Parameters
	ES_Event : the event to process

 Returns
	ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
	
****************************************************************************/
ES_Event RunDrive ( ES_Event ThisEvent )
{
	ES_Event ReturnEvent = {ES_NO_EVENT};
	
	if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == DRIVE_TIMER){
		// Murder the motors
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
		SetPWMDuty(0,STARBOARD_MOTOR);
		SetPWMDuty(0,PORT_MOTOR);
		
		// Repeat if not at correct spot
		if(CheckVal(lastX, 0) &&  CheckVal(lastY, 1)) {
			DriveTo( lastX, lastY );
		}
        else {
            // Post arrived at next point event
			ES_Event newEvent = {AtNextPoint, 0};
			PostMaster(newEvent);
        }
	}
	if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == ROTATE_TIMER){
		// Murder the motors
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
		SetPWMDuty(0,STARBOARD_MOTOR);
		SetPWMDuty(0,PORT_MOTOR);
		
		// Repeat if not at correct angle
		if(CheckVal(lastTheta, 2)) {
			TurnTheta( lastTheta );
		}
        else if(!fromDriveTo) {
            // Post arrived at next angle event
			ES_Event newEvent = {AtNextAngle, 0};
			PostMaster(newEvent);
        }
		fromDriveTo = false;
	}
	
	return ReturnEvent;
}



/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behaviour of this service
*/

void DriveTo( uint16_t X, uint16_t Y ) {
	// Store values of X and Y
	lastX = X;
	lastY = Y;
	
	// Query DRS for current postion, angle
	KART_t myKart = QueryMyKart( );
	
	// Find distance to travel
	float deltaX = X - myKart.KartX;
	float deltaY = Y - myKart.KartY;
	float dist = sqrt(pow(deltaX,2) + pow(deltaY,2));
	
	// Calculate angle
	float theta = atan2(deltaY, deltaX) * 180 / PI; 
	theta = theta - 180;
	if(theta < 0) {
		theta = 360 - theta;
	}
	
	fromDriveTo = true;
	
	// Turn 
	TurnTheta(theta);
	
	// Go straight
	// Set left motor to drive forward
	HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT3HI;
	// Set right motor to drive forward
	HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
	// Set PWM to (FullSpeed)
	SetPWMDuty(FULL_SPEED_PORT,PORT_MOTOR);
	SetPWMDuty(FULL_SPEED_STARBOARD,STARBOARD_MOTOR);
	// Set timer according to theta
	ES_Timer_InitTimer(DRIVE_TIMER,PIXEL_TIME_100*(dist/100));
	// When timeout occurs, murder the motors
}

void TurnTheta( float theta) {
	// Store value of Theta
	lastTheta = theta;
	
	// Query DRS for current angle
	KART_t myKart = QueryMyKart( );
	
	// Find differnece in angles
	float deltaTheta = theta - myKart.KartTheta;
	
	// Rotate
	if(deltaTheta > 0) {
		// Counter-clockwise rotation
		// Set left motor to drive in reverse
		HWREG(GPIO_PORTB_BASE + ALL_BITS) |= BIT3HI;
		// Set right motor to drive forward
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI);
		// Set PWM to (50%)
		SetPWMDuty(100 - HALF_SPEED_PORT,PORT_MOTOR);
		SetPWMDuty(HALF_SPEED_STARBOARD,STARBOARD_MOTOR);
		// Set timer according to theta
		ES_Timer_InitTimer(ROTATE_TIMER,ROTATION_TIME*(theta/360));
		// When timeout occurs, murder the motors
	}
	else if(theta < 0) {
		theta = - theta;
        // Clockwise direction
		// Set left motor to drive forward
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT3HI);
		// Set right motor to drive in reverse
		HWREG(GPIO_PORTB_BASE + ALL_BITS) |= BIT2HI;
		// Set PWM to (50%)
		SetPWMDuty(HALF_SPEED_PORT,PORT_MOTOR);
		SetPWMDuty(100 - HALF_SPEED_STARBOARD,STARBOARD_MOTOR);
		// Set timer according to theta
		ES_Timer_InitTimer(ROTATE_TIMER,ROTATION_TIME*(theta/360));
		// When timeout occurs, murder the motors
	}
}

bool CheckVal(uint16_t val, int select) {
// Checks given value against robot position, return true if within resolution
	// Select: 0 = X, 1 = Y, 2 = Theta
	bool returnVal = false;
	// Query DRS for current postion, angle
	KART_t myKart = QueryMyKart( );
	
	// Return true if value is within resolution of acutal value
	if(select == 0) {
		// Check X value
		float deltaX = val - myKart.KartX;
		if(abs(deltaX) <= PosResolution) {
			returnVal = true;
		}
	}
	if(select == 1) {
		// Check Y value
		float deltaY = val - myKart.KartY;
		if(abs(deltaY) <= PosResolution) {
			returnVal = true;
		}
	}
	if(select == 2) {
		// Check Theta value
		float deltaTheta = val - myKart.KartTheta;
		if(abs(deltaTheta) <= AngleResolution) {
			returnVal = true;
		}
	}
	return returnVal;
}



