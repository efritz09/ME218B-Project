/****************************************************************************
 Module
	Drive.c

 Revision			Revised by: 
	0.1.1				Alex
	0.1.2       Alex
	0.1.3				Alex

 Description
	Drive module initializes PWM and motor pins and provides public functions useful
	for driving the machine

 Edits:
	0.1.1 - Set up as template to outline functionality of driving functions
	0.1.2 - changed to implament for adjustments before arrival at waypoint 
	0.1.3 - added ability to manuver between driving, shooting, and obstacle states
				
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
#include "RunningGame.h"



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
     StartDrive

 Parameters
     ES_Event CurrentEvent

 Returns
     nothing

 Description
     Does any required initialization for this state machine
****************************************************************************/
void StartDrive ( ES_Event CurrentEvent )
{
    // local variable to get debugger to display the value of CurrentEvent
    volatile ES_Event LocalEvent = CurrentEvent;

	fromDriveTo = false;
	
    // use LocalEvent to keep the compiler from complaining about unused var
    RunDrive(LocalEvent);
    return;
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
	ES_Event newEvent = {ES_NO_EVENT, 0};
	
	// Query DRS for current postion, angle
	KART_t myKart = QueryMyKart( );
	currentPoint.X = myKart.KartX;
	currentPoint.Y = myKart.KartY;
	
	// Stop motors in case of emergency or if the next point has been reached
	if(ThisEvent.EventType == AtNextPoint || ThisEvent.EventType == InShootingDecisionZone || 
		ThisEvent.EventType == InObstacleDecisionZone || ThisEvent.EventType == EmergencyStop || 
		ThisEvent.EventType == GameOver){
		
		// Murder the motors
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
		SetPWMDuty(0,STARBOARD_MOTOR);
		SetPWMDuty(0,PORT_MOTOR);
		
		// Eat all timers
		ES_Timer_StopTimer(DRIVE_TIMER);
		ES_Timer_StopTimer(ROTATE_TIMER);
			
	    // Don't drive
	    newEvent.EventType = AtNextPoint;
	}
	if(ThisEvent.EventType == CautionFlagDropped || ThisEvent.EventType == EmergencyStop) {
		
		// Murder the motors
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
		SetPWMDuty(0,STARBOARD_MOTOR);
		SetPWMDuty(0,PORT_MOTOR);
			
	    // Don't drive
	    newEvent.EventType = AtNextPoint;
	}
	// Drive timer time out means that the bot is either at its final location or needs to keep going
	if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == DRIVE_TIMER){
		// Ask if we should stop adjusting our movement towards the target point
		// (within one second of drivingto desired point)	
		if(stopFlag)
		{
			// Murder the motors
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
			SetPWMDuty(0,STARBOARD_MOTOR);
			SetPWMDuty(0,PORT_MOTOR);
			
			newEvent.EventType = AtNextPoint;
			PostMaster(newEvent);
			
			// reset stopFlag
			stopFlag = false;
		}
		// Stop if the point reached is within tolerance of the desired point
		else if((CheckVal(lastX, 0) &&  CheckVal(lastY, 1))) {
			// Murder the motors
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
			SetPWMDuty(0,STARBOARD_MOTOR);
			SetPWMDuty(0,PORT_MOTOR);
			
			newEvent.EventType = AtNextPoint;
			PostMaster(newEvent);
			stopFlag = false;
		}
		// If in the driving state, perform additional checks
		else if(QueryRunningGame( ) == DrivingStateSM ) {
			// If we haven't made a shot yet, check to see if the bot is in the shooting decision zone
			if(!myKart.ShotComplete) {
				// If the bot is in the shooting decision zone, stop and throw an event to indicate
				if(FindDecisionZone(currentPoint) == ShootingDecisionZone) {
					// Murder the motors
					HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
					SetPWMDuty(0,STARBOARD_MOTOR);
					SetPWMDuty(0,PORT_MOTOR);
					
					// Post At Next Point event to move driving SM to at position state
					newEvent.EventType = AtNextPoint;
					newEvent.EventParam = 1;
					PostMaster(newEvent);
					
					// Post At Next Point event to start motion toward shooting point
					ES_Event newEvent1 = {InShootingDecisionZone, 0};
					PostMaster(newEvent1);
					
					stopFlag = false;
				}
			}
			// If we haven't crossed the obstacle yet, check to see if the bot is in the obstacle decision zone
			if(!myKart.ShotComplete) {
				// If the bot is in the obstacle decision zone, stop and throw an event to indicate
				if(FindDecisionZone(currentPoint) == ObstacleDecisionZone) {
					// Murder the motors
					HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
					SetPWMDuty(0,STARBOARD_MOTOR);
					SetPWMDuty(0,PORT_MOTOR);
					
					// Post At Next Point event to move obstacle SM to at position state
					newEvent.EventType = AtNextPoint;
					newEvent.EventParam = 1;
					PostMaster(newEvent);
					
					// Post At Next Point event to start motion toward obstacle point
					ES_Event newEvent1 = {InObstacleDecisionZone, 0};
					PostMaster(newEvent1);
					
					stopFlag = false;
				}
			}
			// If the bot is in the next section, consider it to be the next point 
			// this prevents the bot from turning backward to reach a specific point
			if(FindStraightSection(currentPoint) == nextSection) {
				// Murder the motors
				HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
				SetPWMDuty(0,STARBOARD_MOTOR);
				SetPWMDuty(0,PORT_MOTOR);
				
				newEvent.EventType = AtNextPoint;
				PostMaster(newEvent);
				stopFlag = false;
			}
		}
		if(!(newEvent.EventType == AtNextPoint)) {
			// If none of the previous conditions was met, the bot has work to do
			// Repeat call to drive to the same point as before
			DriveTo(lastX, lastY);
		}
	}
	// Rotate timer timeout means that the bot is finsihed turning
	// this has diffrent meanings based on where the call to rotate came from
	if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == ROTATE_TIMER){
		// Murder the motors
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
		SetPWMDuty(0,STARBOARD_MOTOR);
		SetPWMDuty(0,PORT_MOTOR);
		
		// POTENTIALLY ADD CODE TO REPEAT ANGLES HERE
		//if(CheckVal(lastTheta, 2)) {
			//TurnTheta( lastTheta );
		//}
		
		// Check to the make sure event origionated outside of drive before posting
		if(fromDriveTo) {
					
			printf("Driving \r\n");
			// Go straight
			// Set left motor to drive forward
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT3HI;
			// Set right motor to drive forward
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
			// Set PWM to (1/2 Speed)
			SetPWMDuty(HALF_SPEED_PORT,PORT_MOTOR);
			SetPWMDuty(HALF_SPEED_STARBOARD,STARBOARD_MOTOR);
			
			/* Set drive time:
				if the overall drive time exceeds one second, 
				we would to break up the drive into one second incraments
				this way we can avoid a bad angle reading throwing off our 
				total trajectory
			*/			
			if(driveTime > ONE_SEC/4)
				{
				// Move one second in current theta direction 
				ES_Timer_InitTimer(DRIVE_TIMER,ONE_SEC/4);
			}
			else {
				// Move last step to destination
				ES_Timer_InitTimer(DRIVE_TIMER,driveTime);
				stopFlag = true;
			}
		}
		else {
			// Post arrived at next angle event if not from drive
			ES_Event newEvent = {AtNextAngle, 0};
			PostMaster(newEvent);
		}
		// Reset flag that indicates whether or not the timer originated from a drive call
		fromDriveTo = false;
	}
	return ReturnEvent;
}



/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behaviour of this service
*/

// Drive to given point
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
	
	printf("%d %d %d penis3\n\r", myKart.KartX, myKart.KartY, myKart.KartTheta);
	
	// Find distance to travel
	float deltaX = X - myKart.KartX;
	float deltaY = Y - myKart.KartY; 
	dist = sqrt(pow(deltaX,2) + pow(deltaY,2));
	// Calculate time needed to travel desired distance
	driveTime = dist*3*ONE_SEC/PIXELS_PER_3SEC ; /*modulate by .6 to combat DRS delay*/
	
	
	printf("%f %f penis1\n\r", deltaX, deltaY);
	
	// Calculate angle
	int desiredTheta = abs(atan(abs(deltaY/deltaX)) * 180 / PI); 
	printf("%d penis2\n\r", desiredTheta);
	
	/* Calculate the desired theta desired based on the 
	left handed coordiante system and right handed theta*/
	if(deltaX < 0 && deltaY < 0) {
		desiredTheta = (-1)*desiredTheta;
	}
	else if(deltaX > 0 && deltaY > 0) {
		desiredTheta = 180 - desiredTheta;
	}
	else if(deltaX > 0 && deltaY < 0) {
		desiredTheta = -180 +desiredTheta;
	}
	// Combat case where deltaY is exactly zero
	else if(deltaX > 0 && deltaY == 0) {
		desiredTheta = 180;
	}
	else if(deltaX < 0 && deltaY == 0) {
		desiredTheta = 0;
	}
	
	// Garuntee that the desired theta is between 0 and 360
	if(desiredTheta < 0) {
		desiredTheta = 360 + desiredTheta;
	}
	
	printf("%f %f %d \n\r", dist, dist*3*ONE_SEC/PIXELS_PER_3SEC, desiredTheta);
	
	// Ignore arctan calculated desired theta value if deltaX is zero as this will 
	// lead to a garbage reading from arctan
	if(deltaX == 0) {
		if(deltaY > 0) {
			desiredTheta = 90;
		}
		if(deltaY < 0) {
			desiredTheta = 270;
		}
	}
	
	// Set flag true indicating that the call came from the drive function
	fromDriveTo = true;
	// Call turning function
	TurnTheta(desiredTheta);
}

// Starts a turn timer with the goal of turning to the desired angle
void TurnTheta( float desiredTheta ) {
	printf("Rotating \r\n");
	// Store value of desiredTheta
	lastTheta = desiredTheta;
	
	// Query DRS for current angle
	KART_t myKart = QueryMyKart( );
	
	// Find differnece in angles
	int deltaTheta;
	deltaTheta = desiredTheta - myKart.KartTheta;
	
	printf("Final Angle %d Desired Angle %f \n\r", myKart.KartTheta, desiredTheta);
	
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
		// Set timer according to deltatheta
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
		// Set timer according to deltatheta
		ES_Timer_InitTimer(ROTATE_TIMER,ROTATION_TIME*deltaTheta/360);
		// When timeout occurs, murder the motors
	}
	else if(deltaTheta == 0) {
		// Set very small rotate timer
		ES_Timer_InitTimer(ROTATE_TIMER,1);
	}
	
	printf("%d %d \n\r", deltaTheta, ROTATION_TIME*deltaTheta/360);
}

// Checks given value against robot position, return true if within resolution
bool CheckVal(uint16_t val, int select) {
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

MapSection FindNextSection( POINT_t current ) {
	// Create return map section
	MapSection returnMapSection;
	// Find section of map according to current location
	if( (current.Y >= y2 && current.X <= x2) || ((current.Y >= 96 && current.Y <= 150) && (current.X >= 70 && current.X <= 146))) {
		printf("In Bottom Straight \r\n");
		returnMapSection = RightStraight;
	}
	else if((current.Y >= y1 && current.X >= x2) || ((current.Y >= 96 && current.Y <= 150) && (current.X >= 146 && current.X <= 222))) {
		printf("In Right Straight \r\n");
		returnMapSection = TopStraight;
	}
	else if((current.Y <= y1 && current.X >= x1) || ((current.Y >= 42 && current.Y <= 96) && (current.X >= 146 && current.X <= 222))) {
		printf("In Top Straight \r\n");
		returnMapSection = LeftStraight;
	}
	else if((current.Y <= y2 && current.X <= 70) || ((current.Y >= 42 && current.Y <= 96) && (current.X >= 70 && current.X <= 146))) {
		printf("In Left Straight \r\n");
		returnMapSection = BottomStraight;
	}
		
	return returnMapSection;
}


