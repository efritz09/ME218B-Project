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
#include "Headers.h"

/*----------------------------- Module Defines ----------------------------*/
#define PI 3.14159265
#define PosResolution 10
#define AngleResolution 10


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint16_t lastCommandedX;
static uint16_t lastCommandedY;
static uint16_t lastCommandedTheta;
static float dist;
static int driveTime;
static MapSection nextSection;
static POINT_t currentPoint;
static bool counterClockwiseRotate = false;
static uint16_t thetaTime;

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
		ThisEvent.EventType == InObstacleDecisionZone || ThisEvent.EventType == GameOver){
		
		// Murder the motors
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
		SetPWMDuty(0,STARBOARD_MOTOR);
		SetPWMDuty(0,PORT_MOTOR);
		
		// Eat all timers
		ES_Timer_StopTimer(DRIVE_TIMER);
		ES_Timer_StopTimer(ROTATE_TIMER);
	
	}
	if(ThisEvent.EventType == CautionFlagDropped || ThisEvent.EventType == EmergencyStop) {
		
		// Murder the motors
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
		SetPWMDuty(0,STARBOARD_MOTOR);
		SetPWMDuty(0,PORT_MOTOR);
		
	}
	// Drive timer time out means that the bot is at its final location 
	if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == DRIVE_TIMER){
			// Murder the motors
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
			SetPWMDuty(0,STARBOARD_MOTOR);
			SetPWMDuty(0,PORT_MOTOR);
			
			newEvent.EventType = AtNextPoint;
			PostMaster(newEvent);
	}
	// Rotate timer timeout means that the bot is finsihed turning
	// this has diffrent meanings based on where the call to rotate came from
	if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == ROTATE_TIMER){
		// Murder the motors
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
		SetPWMDuty(0,STARBOARD_MOTOR);
		SetPWMDuty(0,PORT_MOTOR);
		
		// POTENTIALLY ADD CODE TO REPEAT ANGLES HERE
		//if(CheckVal(lastCommandedTheta, 2)) {
			//TurnTheta( lastCommandedTheta );
		//}
		
		// Post arrived at next angle 
			ES_Event newEvent = {AtNextAngle, 0};
			PostMaster(newEvent);

	}
	return ReturnEvent;
}



/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behaviour of this service
*/

void Calculate ( uint16_t X, uint16_t Y ) {
	lastCommandedX = X;
	lastCommandedY = Y;
	printf("Desired Coordinates: %d %d \n\r", X, Y);
	
	// Query DRS for current postion, angle
	KART_t myKart = QueryMyKart( );
	
	// Find next section
	currentPoint.X = myKart.KartX;
	currentPoint.Y = myKart.KartY;
	
	printf("Current Kart Data X: %d Y: %d Theta: %d \n\r", myKart.KartX, myKart.KartY, myKart.KartTheta);
	
	// Find distance to travel
	float deltaX = X - myKart.KartX;
	float deltaY = Y - myKart.KartY; 
	dist = sqrt(pow(deltaX,2) + pow(deltaY,2));
	// Calculate time needed to travel desired distance
	driveTime = dist*3*ONE_SEC/PIXELS_PER_3SEC ; /*modulate by .6 to combat DRS delay*/
	
	printf("deltaX: %f deltaY: %f penis1\n\r", deltaX, deltaY);
	
	// Test values
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
	//printf("Arctan Output: %d \n\r", abs(atan(abs(142/)) * 180 / PI));
	printf("Arctan Output: %d \n\r", desiredTheta);
	
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
	
	// Store value of desiredTheta
	lastCommandedTheta = desiredTheta;
	
	// Find differnece in angles
	int deltaTheta;
	deltaTheta = desiredTheta - myKart.KartTheta;//getDesiredTheta();
	
	printf("Vector Calculations Dist: %f  Desired Theta: %d \n\r", dist, desiredTheta);
	
	//garuntee that the bot never rotates more than 180 degrees
	if(deltaTheta > 180) {
		deltaTheta = deltaTheta - 360;
	}
	if(deltaTheta < (-1)*180) {
		deltaTheta = deltaTheta + 360;
	}

	printf("deltaTheta: %d \n\r", deltaTheta);
	
	// Rotate
	if(deltaTheta > 0) {
		// Counter-clockwise rotation
		counterClockwiseRotate = true;
		// Set timer according to deltatheta
		thetaTime = ROTATION_TIME*deltaTheta/360;
		// When timeout occurs, murder the motors
	}
	else if(deltaTheta < 0) {
		deltaTheta = abs(deltaTheta);
    // Clockwise direction
		counterClockwiseRotate = false;
		thetaTime = ROTATION_TIME*deltaTheta/360;
		// When timeout occurs, murder the motors
	}
	else if(deltaTheta == 0) {
		// Set very small rotate timer
		thetaTime = 1;
	}
	printf("Drive Time:%d Theta Time: %d \n\r", driveTime, thetaTime);
	
	if(driveTime > ONE_SEC/2) {
		driveTime = ONE_SEC/2;
	}
	else {
		//stopFlag = true;
	}
	if (FindSection(currentPoint) == TopStraight) {
		thetaTime += 40;
	}
		if (FindSection(currentPoint) == RightStraight) {
		thetaTime += 40;
	}
	driveTime = driveTime*0.9;
	
	// Find next section
	nextSection = FindNextSection(FindSection(currentPoint));
	
	ES_Event newEvent = {PathGenerated, 0};
	PostMaster(newEvent);
}

// Kick out 45 degrees relitive to axis of region
void ExitZoneCalc (int zoneNumber) {
	int exitZoneTheta;
	// Desired theta is calculated based upon the current zone
	switch(zoneNumber) {
		case 1:
			// Bin
			exitZoneTheta = 135;
		case 2:
			// Rin
			exitZoneTheta = 225;
		case 3:
			// Tin
			exitZoneTheta = 315;
		case 4:
			// Lin
			exitZoneTheta = 45;
	}
	
	KART_t myKart = QueryMyKart();
	
	// Find differnece in angles
	int deltaTheta;
	deltaTheta = exitZoneTheta - myKart.KartTheta;//getDesiredTheta();
	
	printf("Exit Zone Theta: %d \n\r", exitZoneTheta);
	
	//garuntee that the bot never rotates more than 180 degrees
	if(deltaTheta > 180) {
		deltaTheta = deltaTheta - 360;
	}
	if(deltaTheta < (-1)*180) {
		deltaTheta = deltaTheta + 360;
	}

	printf("deltaTheta: %d \n\r", deltaTheta);
	
	// Rotate
	if(deltaTheta > 0) {
		// Counter-clockwise rotation
		counterClockwiseRotate = true;
		// Set timer according to deltatheta
		thetaTime = ROTATION_TIME*deltaTheta/360;
		// When timeout occurs, murder the motors
	}
	else if(deltaTheta < 0) {
		deltaTheta = abs(deltaTheta);
    // Clockwise direction
		counterClockwiseRotate = false;
		thetaTime = ROTATION_TIME*deltaTheta/360;
		// When timeout occurs, murder the motors
	}
	else if(deltaTheta == 0) {
		// Set very small rotate timer
		thetaTime = 1;
	}
	
	// Choose a 1 second drive time arbitratily since there is no destinaiton point
	driveTime = ONE_SEC;
	
	printf("Drive Time:%d Theta Time: %d \n\r", driveTime, thetaTime);
	
	ES_Event newEvent = {PathGenerated, 0};
	PostMaster(newEvent);
}

// Drive to given point
void DriveForward( void ) {
	clearThetas();
	
	printf("Driving \r\n");
	// Set Motors forward, drive for length of drive timer
	HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT3HI;
	HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
	// Set PWM to (HalfSpeed)
	SetPWMDuty(HALF_SPEED_PORT,PORT_MOTOR);
	SetPWMDuty(HALF_SPEED_STARBOARD,STARBOARD_MOTOR);
	
	// Start timer for drive time
  ES_Timer_InitTimer(DRIVE_TIMER,driveTime);
	
}

// Starts a turn timer with the goal of turning to the desired angle
void TurnTheta( void ) {
	printf("Rotating \r\n");
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
		
		// Set PWM to (HalfSpeed)
		SetPWMDuty(HALF_SPEED_PORT,PORT_MOTOR);
		SetPWMDuty(HALF_SPEED_STARBOARD,STARBOARD_MOTOR);
		// Start timer for drive time
		ES_Timer_InitTimer(ROTATE_TIMER,thetaTime);
	
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

