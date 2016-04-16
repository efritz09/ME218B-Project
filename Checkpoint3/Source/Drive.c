/****************************************************************************
 Module
	Drive.c

 Revision			Revised by: 
	0.1.1				Alex
	0.1.2       Alex

 Description
	Drive module initializes PWM and motor pins and provides public functions useful
	for driving the machine

 Edits:
	0.1.1 - Set up as template to outline functionality of driving functions
	0.1.2 - changed to implament for adjustments before arrival at waypoint 
				
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
#include "Driving.h"



/*----------------------------- Module Defines ----------------------------*/
#define BitsPerNibble  4
#define TicksPerMS 40000
#define ALL_BITS (0xff<<2)
#define PI 3.14159265

#define PosResolution 10
#define AngleResolution 10


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static uint16_t lastX;
static uint16_t lastY;
static uint16_t lastTheta;
static bool fromDriveTo = false;
static float dist;
static int driveTime;
static bool stopFlag;
static MapSection nextSection;
static POINT_t currentPoint;

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
	
	
	if(ThisEvent.EventType == AtNextPoint || 
		ThisEvent.EventType == CautionFlagDropped ||
	ThisEvent.EventType == EmergencyStop || 
	ThisEvent.EventType == GameOver){
		// Murder the motors
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
		SetPWMDuty(0,STARBOARD_MOTOR);
		SetPWMDuty(0,PORT_MOTOR);
		
		// Eat all timers
		ES_Timer_StopTimer(DRIVE_TIMER);
		ES_Timer_StopTimer(ROTATE_TIMER);
	}
	if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == DRIVE_TIMER){
		if(stopFlag)/*asks if we should stop adjusting our movement towards the target point
			(within one second of drivingto desired point)*/ 
		{
			// Murder the motors
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
			SetPWMDuty(0,STARBOARD_MOTOR);
			SetPWMDuty(0,PORT_MOTOR);
			
			ES_Event newEvent = {AtNextPoint, 0};
			PostMaster(newEvent);
			
			// reset stopFlag
			stopFlag = false;
		}
		else if((CheckVal(lastX, 0) &&  CheckVal(lastY, 1))) {
			// Murder the motors
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
			SetPWMDuty(0,STARBOARD_MOTOR);
			SetPWMDuty(0,PORT_MOTOR);
			
			ES_Event newEvent = {AtNextPoint, 0};
			PostMaster(newEvent);
			stopFlag = false;
		}
		else if(FindSection(currentPoint) == nextSection) {
			// Murder the motors
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
			SetPWMDuty(0,STARBOARD_MOTOR);
			SetPWMDuty(0,PORT_MOTOR);
			
			ES_Event newEvent = {AtNextPoint, 0};
			PostMaster(newEvent);
			stopFlag = false;
		}
		else {
			//if we are not at the point recall attempt at going to the point
			DriveTo(lastX, lastY);
		}
		
		//printf("%d %d \n\r", CheckVal(lastX, 0), CheckVal(lastY, 1));
		
		// Repeat if not at correct spot
		//if(!(CheckVal(lastX, 0) &&  CheckVal(lastY, 1))) {
			//DriveTo( lastX, lastY );
		//}
       // else {
      // Post arrived at next point event
			
        //}
	}
	if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == ROTATE_TIMER){
		// Murder the motors
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
		SetPWMDuty(0,STARBOARD_MOTOR);
		SetPWMDuty(0,PORT_MOTOR);
		
		// Repeat if not at correct angle
		//if(CheckVal(lastTheta, 2)) {
			//TurnTheta( lastTheta );
		//}
    if(!fromDriveTo)/*checks the make sure event origionated outside of drive before posting*/
			{
            // Post arrived at next angle event if not from drive
			ES_Event newEvent = {AtNextAngle, 0};
			PostMaster(newEvent);
        }
		fromDriveTo = false;
				
				printf("Driving \r\n");
	// Go straight
	// Set left motor to drive forward
	HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT3HI;
	// Set right motor to drive forward
	HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
	// Set PWM to (1/2 Speed)
	SetPWMDuty(HALF_SPEED_PORT,PORT_MOTOR);
	SetPWMDuty(HALF_SPEED_STARBOARD,STARBOARD_MOTOR);
		
/*if the overall drive time exceeds one second, 
				we would to break up the drive into one second incraments
				this way we can avoid a bad angle reading throwing off our 
				total trajectory
				*/		
				
	if(driveTime > ONE_SEC)
		{
		// move one second in current theta direction 
		ES_Timer_InitTimer(DRIVE_TIMER,ONE_SEC);
	}
	else {
		// move last step to destination
		ES_Timer_InitTimer(DRIVE_TIMER,driveTime);
		stopFlag = true;
	}
		
	// When timeout occurs, murder the motors
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
	
	printf("%d %d \n\r", X, Y);
	
	// Query DRS for current postion, angle
	KART_t myKart = QueryMyKart( );
	
	// Find next section
	currentPoint.X = myKart.KartX;
  currentPoint.Y = myKart.KartY;
	nextSection = FindNextSection(currentPoint);
	
	printf("%d %d %d 1\n\r", myKart.KartX, myKart.KartY, myKart.KartTheta);
	
	// Find distance to travel
	float deltaX = X - myKart.KartX;
	float deltaY = Y - myKart.KartY; 
	dist = sqrt(pow(deltaX,2) + pow(deltaY,2));
	// calculate time needed to travel desired distance
	driveTime = dist*3*ONE_SEC/PIXELS_PER_3SEC * 0.6;/*modulate by .6 to combat DRS delay*/
	
	
	printf("%f %f 2\n\r", deltaX, deltaY);
	
	//account for delta x equals 0
	
	// Calculate angle
	int theta = abs(atan(abs(deltaY/deltaX)) * 180 / PI); 
	printf("%d 3\n\r", theta);
	
	/* calculate the theta desired based on the 
	left handed coordiante system and right handed theta*/
	
	if(deltaX < 0 && deltaY < 0) {
		theta = (-1)*theta;
	}
	else if(deltaX > 0 && deltaY > 0) {
		theta = 180 - theta;
	}
	else if(deltaX > 0 && deltaY < 0) {
		theta = -180 +theta;
	}
	
	// garuntee theta is between 0 and 360
	if(theta < 0) {
		theta = 360 + theta;
	}
	
	printf("%f %f %d \n\r", dist, dist*3*ONE_SEC/PIXELS_PER_3SEC, theta);
	
	fromDriveTo = true;
	
	if(deltaX == 0) {
		if(deltaY > 0) {
			theta = 90;
		}
		if(deltaY < 0) {
			theta = 270;
		}
	}
	
	// Turn 
	TurnTheta(theta);
}

void TurnTheta( float theta) {
	printf("Rotating \r\n");
	// Store value of Theta
	lastTheta = theta;
	
	// Query DRS for current angle
	KART_t myKart = QueryMyKart( );
	
	
	// Find differnece in angles
	int deltaTheta;
	deltaTheta = theta - myKart.KartTheta;
	
	//garuntee that the bot never rotates more than 180 degrees
	if(deltaTheta > 180) {
		deltaTheta = deltaTheta - 360;
	}
	if(deltaTheta < (-1)*180) {
		deltaTheta = deltaTheta + 360;
	}

	
	printf("%d %d \n\r", deltaTheta, ROTATION_TIME*deltaTheta/360);
	
	// Rotate
	if(deltaTheta > 0) {
		// Counter-clockwise rotation
		// Set left motor to drive in reverse
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~BIT3HI;
		// Set right motor to drive forward
		HWREG(GPIO_PORTB_BASE + ALL_BITS) |= BIT2HI;
		// Set PWM to (50%)
		SetPWMDuty(HALF_SPEED_PORT,PORT_MOTOR);
		SetPWMDuty(HALF_SPEED_STARBOARD,STARBOARD_MOTOR);
		// Set timer according to theta
		ES_Timer_InitTimer(ROTATE_TIMER,ROTATION_TIME*deltaTheta/360);
		// When timeout occurs, murder the motors
	}
	else if(deltaTheta < 0) {
		deltaTheta = abs(deltaTheta);
        // Clockwise direction
		// Set left motor to drive forward
		HWREG(GPIO_PORTB_BASE + ALL_BITS) |= BIT3HI;
		// Set right motor to drive in reverse
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~BIT2HI;
		// Set PWM to (50%)
		SetPWMDuty(HALF_SPEED_PORT,PORT_MOTOR);
		SetPWMDuty(HALF_SPEED_STARBOARD,STARBOARD_MOTOR);
		// Set timer according to theta
		ES_Timer_InitTimer(ROTATE_TIMER,ROTATION_TIME*deltaTheta/360);
		// When timeout occurs, murder the motors
	}
	else if(deltaTheta == 0) {
		// Go straight
		// Set left motor to drive forward
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT3HI;
		// Set right motor to drive forward
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
		// Set PWM to (1/2 Speed)
		SetPWMDuty(HALF_SPEED_PORT,PORT_MOTOR);
		SetPWMDuty(HALF_SPEED_STARBOARD,STARBOARD_MOTOR);
				
			/*if the overall drive time exceeds two seconds, 
				we would to break up the drive into one second incraments
				this way we can avoid a bad angle reading throwing off our 
				total trajectory
				*/
		if(driveTime > ONE_SEC*2) {
			// Set timer according to theta
			ES_Timer_InitTimer(DRIVE_TIMER,ONE_SEC*2);
		}
		else {
			ES_Timer_InitTimer(DRIVE_TIMER,driveTime);
			stopFlag = true;
		}
	}
	
	
	printf("%d %d \n\r", deltaTheta, ROTATION_TIME*deltaTheta/360);
	
}

bool CheckVal(uint16_t val, int select) {
// Checks given value against robot position, return true if within resolution
	// Select: 0 = X, 1 = Y, 2 = Theta
	bool returnVal = false;
	// Query DRS for current postion, angle
	KART_t myKart = QueryMyKart( );
	

	currentPoint.X = myKart.KartX;
  currentPoint.Y = myKart.KartY;
	
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

MapSection FindNextSection( POINT_t current ) {
	// Create return map section
	MapSection returnMapSection;
	// Find section of map according to current location
	if( (current.Y >= 150 && current.X <= 222) || ((current.Y >= 96 && current.Y <= 150) && (current.X >= 70 && current.X <= 146))) {
		printf("In Bottom Straight \r\n");
		returnMapSection = RightStraight;
	}
	else if((current.Y >= 42 && current.X >= 222) || ((current.Y >= 96 && current.Y <= 150) && (current.X >= 146 && current.X <= 222))) {
		printf("In Right Straight \r\n");
		returnMapSection = TopStraight;
	}
	else if((current.Y <= 42 && current.X >= 70) || ((current.Y >= 42 && current.Y <= 96) && (current.X >= 146 && current.X <= 222))) {
		printf("In Top Straight \r\n");
		returnMapSection = LeftStraight;
	}
	else if((current.Y <= 150 && current.X <= 70)  || ((current.Y >= 42 && current.Y <= 96) && (current.X >= 70 && current.X <= 146))) {
		printf("In Left Straight \r\n");
		returnMapSection = BottomStraight;
	}
		
	return returnMapSection;
}


