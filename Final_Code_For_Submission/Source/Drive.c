/****************************************************************************
 Module
	Drive.c

 Revision			Revised by: 
	0.1.1				Alex
	0.1.2       Alex
	0.1.3				Alex
	0.1.4				Denny

 Description
	Drive module initializes PWM and motor pins and provides public functions useful
	for driving the machine

 Edits:
	0.1.1 - Set up as template to outline functionality of driving functions
	0.1.2 - changed to implament for adjustments before arrival at waypoint 
	0.1.3 - added ability to manuver between driving, shooting, and obstacle states
	0.1.4 - added banking turns for small angle adjustments while driving straight
****************************************************************************/
// If we are debugging and setting our own Game/KART states
#define TEST

/*----------------------------- Include Files -----------------------------*/
#include "Headers.h"

/*----------------------------- Module Defines ----------------------------*/
#define PI 3.14159265
#define PosResolution 10
#define AngleResolution 10

/*---------------------------- Module Variables ---------------------------*/
static float dist;
static int driveTime;
static POINT_t currentPoint;
static bool counterClockwiseRotate = false;
static uint16_t thetaTime;
static uint16_t turningTheta;

/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
     StartDrive

 Parameters
     ES_Event CurrentEvent

 Returns
     nothing

 Description
     Passes initial event to RunDrive
****************************************************************************/
void StartDrive ( ES_Event CurrentEvent )
{
    // local variable to get debugger to display the value of CurrentEvent
    volatile ES_Event LocalEvent = CurrentEvent;

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
	Posts events when the robot is done rotating and driving forward
****************************************************************************/
ES_Event RunDrive ( ES_Event ThisEvent )
{
	ES_Event ReturnEvent = {ES_NO_EVENT};
	ES_Event newEvent = {ES_NO_EVENT, 0};
	
	// Query DRS for current postion, angle
	KART_t myKart = QueryMyKart( );
	currentPoint.X = myKart.KartX;
	currentPoint.Y = myKart.KartY;
	
	// Stop motors in case of emergency
	if(ThisEvent.EventType == GameOver){	
		// Murder the motors
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
		SetPWMDuty(0,STARBOARD_MOTOR);
		SetPWMDuty(0,PORT_MOTOR);

		// Eat all timers
		ES_Timer_StopTimer(DRIVE_TIMER);
		ES_Timer_StopTimer(ROTATE_TIMER);
	}
	// Drive timer time out means that the bot is at its final location 
	if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == DRIVE_TIMER){
		newEvent.EventType = AtNextPoint;
		PostMaster(newEvent);
	}
	// Rotate timer timeout means that the bot is finsihed turning
	if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == ROTATE_TIMER){
		// Post arrived at next angle 
		ES_Event newEvent = {AtNextAngle, 0};
		PostMaster(newEvent);
	}
	return ReturnEvent;
}

/*---------------------------- Module Functions ---------------------------*/

// Calculate drive time and rotate time required to move to a given point from current position
void Calculate ( uint16_t X, uint16_t Y ) {
	printf("Desired Coordinates: %d %d \n\r", X, Y);
	
	// Query DRS for current postion, angle
	KART_t myKart = QueryMyKart( );
	currentPoint.X = myKart.KartX;
	currentPoint.Y = myKart.KartY;
	
	printf("Current Kart Data X: %d Y: %d Theta: %d \n\r", myKart.KartX, myKart.KartY, myKart.KartTheta);
	
	// Find distance to travel
	float deltaX = X - myKart.KartX;
	float deltaY = Y - myKart.KartY; 
	dist = sqrt(pow(deltaX,2) + pow(deltaY,2));
	// Calculate time needed to travel desired distance
	driveTime = dist*3*ONE_SEC/PIXELS_PER_3SEC ;
	
	printf("deltaX: %f deltaY: %f penis1\n\r", deltaX, deltaY);
	
	// Take absolute value of deltaX and deltaY to ensure accurate angle calculations 
	float absDeltaX = deltaX;
	float absDeltaY = deltaY;
	
	if(deltaX < 0) {
		absDeltaX = (-1)*deltaX;
	}
	if(deltaY < 0) {
		absDeltaY = (-1)*deltaY;
	}
	
	// Calculate angle
	int desiredTheta = abs(atan((absDeltaY)/(absDeltaX)) * 180 / PI); 
	printf("Arctan Output: %d \n\r", desiredTheta);
	
	// Calculate the desired theta desired based on the 
	// left handed coordiante system and right handed theta*/
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
	
	// Find differnece in angles
	int deltaTheta;
	deltaTheta = desiredTheta - myKart.KartTheta;
	
	printf("Vector Calculations Dist: %f  Desired Theta: %d \n\r", dist, desiredTheta);
	
	// Garuntee that the bot never rotates more than 180 degrees
	if(deltaTheta > 180) {
		deltaTheta = deltaTheta - 360;
	}
	if(deltaTheta < (-1)*180) {
		deltaTheta = deltaTheta + 360;
	}

	printf("deltaTheta: %d \n\r", deltaTheta);
	
	// Calculate the time needed to rotate deltaTheta
	if(deltaTheta > 0) {
		// Counter-clockwise rotation
		counterClockwiseRotate = true;
		// Set timer according to deltaTheta
		thetaTime = ROTATION_TIME*deltaTheta/360;
	}
	else if(deltaTheta < 0) {
		deltaTheta = abs(deltaTheta);
    // Clockwise direction
		counterClockwiseRotate = false;
		// Set timer according to deltaTheta
		thetaTime = ROTATION_TIME*deltaTheta/360;
	}
	else if(deltaTheta == 0) {
		// Set very small rotate timer
		thetaTime = 0;
	}
	
	// Save magnitude of theta to turn
	turningTheta = deltaTheta;
	
	printf("Drive Time:%d Theta Time: %d \n\r", driveTime, thetaTime);
	
	// Limit all driving to half a second at most
	if(driveTime > ONE_SEC/2) {
		driveTime = ONE_SEC/2;
	}
	
	// Manual corrections for places in which turns were too shallow
	if (FindSection(currentPoint) == TopStraight) {
		thetaTime += 40;
	}
		if (FindSection(currentPoint) == RightStraight) {
		thetaTime += 40;
	}
	
	// Modulate drive time to account for delay in DRS readings
	driveTime = driveTime*0.9;
	
	// Post that path generation is complete
	ES_Event newEvent = {PathGenerated, 0};
	PostMaster(newEvent);
}

// Starts drive timer to drive straight for desired amount of time
void DriveForward( void ) {
	// Clear moving average used to smooth theta from DRS
	clearThetas();
	printf("Driving \r\n");
	
	// Check if we need to recalculate
	if( driveTime == 0 ) {
		ES_Event newEvent = {ES_TIMEOUT,DRIVE_TIMER};
		PostMaster(newEvent);
	}
	// Otherwise drive forward for calculated time
	else {
		// Set Motors forward, drive for length of drive timer
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT3HI;
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
		// Set PWM to (HalfSpeed)
		SetPWMDuty(HALF_SPEED_PORT,PORT_MOTOR);
		SetPWMDuty(HALF_SPEED_STARBOARD,STARBOARD_MOTOR);
	
		// Start timer for drive time
		ES_Timer_InitTimer(DRIVE_TIMER,driveTime);
	}
}

// Starts a turn timer with the goal of turning to the desired angle
void TurnTheta( void ) {
	printf("Rotating - ");
	
	// Determine which type of turn should be used
	// No turn
	if( turningTheta < 5 ) {
		ES_Event newEvent = {ES_TIMEOUT, ROTATE_TIMER};
		PostMaster(newEvent);
	}
	// Banked turn
	else if( turningTheta < 20) {
		printf("Bank Turn\r\n");
		// Adjust theta time for bank turning
		thetaTime = ((BANK_90_DEGREE_TIME/90)*turningTheta)*0.9;
		// Set drive forward time to 0 so we recalculate at end of bank turn
		driveTime = 0;
		
		// Check direction of turn
		if(counterClockwiseRotate) {
			// Set to turn counter-clockwise, drive for length of drive timer
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT3HI;
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
			
			// Set the PWM duty for CCW Bank Turn
			SetPWMDuty(QUARTER_SPEED_PORT,PORT_MOTOR);
			SetPWMDuty(HALF_SPEED_STARBOARD,STARBOARD_MOTOR);
		}
		else {
			// Set to turn clockwise, drive for length of drive timer
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT3HI;
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
			
			// Set the PWM duty for CW Bank Turn
			SetPWMDuty(HALF_SPEED_PORT,PORT_MOTOR);
			SetPWMDuty(QUARTER_SPEED_STARBOARD,STARBOARD_MOTOR);
		}
		
		// Start timer for drive time
		ES_Timer_InitTimer(ROTATE_TIMER,thetaTime);
	}
	// Full turn
	else {
		printf("Reverse Turn\r\n");
		// Check direction of turn
		if(counterClockwiseRotate) {
			// Set to turn counter-clockwise, drive for length of drive timer
			HWREG(GPIO_PORTB_BASE + ALL_BITS) |= BIT3HI;
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
		}
		else {
			// Set to turn clockwise, drive for length of drive timer
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~BIT3HI;
			HWREG(GPIO_PORTB_BASE + ALL_BITS) |=  BIT2HI;
		}
		
		// Set PWM to (HalfSpeed) for both motors
		SetPWMDuty(HALF_SPEED_PORT,PORT_MOTOR);
		SetPWMDuty(HALF_SPEED_STARBOARD,STARBOARD_MOTOR);
		
		// Start timer for drive time
		ES_Timer_InitTimer(ROTATE_TIMER,thetaTime);
	}
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
		if(abs(deltaX) <= 30) {
			returnVal = true;
		}
	}
	if(select == 1) {
		// Check Y value
		float deltaY = val - myKart.KartY;
		if(abs(deltaY) <= 50) {
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

