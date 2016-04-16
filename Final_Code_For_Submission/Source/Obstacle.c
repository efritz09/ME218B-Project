/****************************************************************************
 Module
   Obstacle.c

 Revision			Revised by: 
	0.1.1				Alex
	0.2.1				Alex
	0.3.1				Alex
	0.4.1				Alex

 Description
	Obstacle crossing state machine that controls traversing the obstacle
	
	could add swinging motion when finding line to make sure we get there

 Edits:
	0.1.1 - Set up as template 
	0.2.1 - Update obstacle state machine for tape finding with two analog tape sensors
	0.3.1 - Update to use timers to cross the obstacle
	0.3.1 - Use drive type system to control movement toward end of obstacle
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
#include "Headers.h"

/*----------------------------- Module Defines ----------------------------*/
#define OBSTACLE_ORIENTATION 270

#define SLIGHT_RIGHT_PORT HALF_SPEED_PORT - 5
#define SLIGHT_RIGHT_STARBOARD HALF_SPEED_STARBOARD + 5

#define SLIGHT_LEFT_PORT HALF_SPEED_PORT + 5
#define SLIGHT_LEFT_STARBOARD HALF_SPEED_STARBOARD - 5

#define TIME_TO_OEPOINT 200 //milisec
#define TIME_TO_TEETERPOINT (1.5*ONE_SEC) //milisec

#define X_CheckTime 100
#define Y_CheckTime 100

/*---------------------------- Module Functions ---------------------------*/
static ES_Event DuringOrientState( ES_Event Event ); 
static ES_Event DuringFindXState( ES_Event Event ); 
static ES_Event DuringObstacleTurning1State( ES_Event Event );
static ES_Event DuringFindYState( ES_Event Event );
static ES_Event DuringObstacleGeneratePathState( ES_Event Event );
static ES_Event DuringObstacleTurningState( ES_Event Event );
static ES_Event DuringObstacleDrivingForwardState( ES_Event Event );
static ES_Event DuringExitingObstacleState( ES_Event Event );

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static ObstacleState CurrentState;
KART_t myKart_Obstacle;
POINT_t ObstacleExit = {181, 40};

/*------------------------------ Module Code ------------------------------*/

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
	CurrentState = Orient;
	
   // Call the entry function (if any) for the ENTRY_STATE
   RunObstacle(CurrentEvent);
}

/****************************************************************************
 Function
   RunObstacle

 Parameters
   unsigned char: the event to process

 Returns
   unsigned char: an event to return

 Description
   implements obstacle state machine using DRS data
****************************************************************************/
ES_Event RunObstacle( ES_Event CurrentEvent )
{
	bool MakeTransition = false;/* are we making a state transition? */
	ObstacleState NextState = CurrentState;
	ES_Event ReturnEvent = CurrentEvent; // assume we are not consuming event

	// Query DRS for current position and orientation
	myKart_Obstacle = QueryMyKart();

	switch ( CurrentState )
	{
		case Orient:
			// Execute during function
			CurrentEvent = DuringOrientState(CurrentEvent);
			// Process events
			if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
			{
				switch (CurrentEvent.EventType)
				{
					case ES_TIMEOUT:
						// Move forward if the bot is at around the correct X value
						if(CurrentEvent.EventParam == OBS_TIMER) {
							NextState = FindX;
							MakeTransition = true;
							printf("properly oriented\r\n");
							// Murder the motors
							HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
							SetPWMDuty(0,STARBOARD_MOTOR);
							SetPWMDuty(0,PORT_MOTOR);
						}
						break;
				}
			}				                
			break;
	  case FindX:
			// Execute during function
			CurrentEvent = DuringFindXState(CurrentEvent);
			// Process events
			if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
			{
				switch (CurrentEvent.EventType)
				{
					case ES_TIMEOUT:
						// Move forward if the bot is at around the correct X value
						if(CurrentEvent.EventParam == OBS_TIMER) {
							if(myKart_Obstacle.KartX >= X_O) {
								printf("At Correct X \r\n");
								NextState = Turning;
								MakeTransition = true;
							}
							else {
								// Re-intitialize timer
								ES_Timer_InitTimer(OBS_TIMER,X_CheckTime);
							}
						}
						break;
				}
			}				                
			break;
		case Turning:
			// Execute during function
			CurrentEvent = DuringObstacleTurning1State(CurrentEvent);
			// Process events
			if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
			{
				switch (CurrentEvent.EventType)
				{
					case ES_TIMEOUT:
						// Move forward if the bot is at around the correct X value
						if(CurrentEvent.EventParam == OBS_TIMER) {
							printf("At 270 \r\n");
							NextState = FindY;
							MakeTransition = true;
						}
				}
			}
			break;
		case FindY:
			// Execute during function
			CurrentEvent = DuringFindYState(CurrentEvent);
			// Process events
			if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
			{
				switch (CurrentEvent.EventType)
				{
					case ES_TIMEOUT: //If event is event one
						if(CurrentEvent.EventParam == NITRO_TIMER) {
							// Set left motor to drive forward
							printf("Nitro Timer \r\n");
							HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT3HI;
							// Set right motor to drive forward
							HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
							// Set PWM to (1/4 Speed)
							SetPWMDuty(QUARTER_SPEED_PORT+10,PORT_MOTOR);
							SetPWMDuty(QUARTER_SPEED_STARBOARD+10,STARBOARD_MOTOR);
							ES_Timer_InitTimer(OBS_TIMER,X_CheckTime);	 
						} 
						else if(CurrentEvent.EventParam == OBS_TIMER) {
							printf("At Beginging of obstacle \r\n");										
							if(myKart_Obstacle.KartY < (y2+10)) {
								printf("At Correct Y \r\n");
								NextState = ObstacleGeneratePathState;
								// Murder the motors
								HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
								SetPWMDuty(0,STARBOARD_MOTOR);
								SetPWMDuty(0,PORT_MOTOR);
								MakeTransition = true;
							}
							else {
								// Re-intitialize timer
								ES_Timer_InitTimer(OBS_TIMER,Y_CheckTime);
							}
						}
						break;
				}
			}
			break;
		case ObstacleGeneratePathState:
			// Execute during function
			CurrentEvent = DuringObstacleGeneratePathState(CurrentEvent);
			// Process events
			if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
			{
				switch (CurrentEvent.EventType)
				{
					case PathGenerated:
						printf("Path Generated \r\n");
						// Move within the obstacle SM
						NextState = ObstacleTurningState;
						MakeTransition = true;
						break;
				}
			}
			break;
		case ObstacleTurningState:
			// Execute during function
			CurrentEvent = DuringObstacleTurningState(CurrentEvent);
			// Process events
			if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
			{
				switch (CurrentEvent.EventType)
				{
					case AtNextAngle:
						printf("At Next Angle \r\n");
						// Move within the driving SM
						NextState = ObstacleDrivingForwardState;
						MakeTransition = true;
						break;
				}
			}
			break;
		case ObstacleDrivingForwardState:
			// Execute during function
			CurrentEvent = DuringObstacleDrivingForwardState(CurrentEvent);
			// Process events
			 
			if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
			{
				switch (CurrentEvent.EventType)
				{				
					case AtNextPoint:
						printf("At Next Point \r\n");		
						if(myKart_Obstacle.KartY <= y1+20) {
							// Move within the driving SM
							NextState = ExitingObstacle;
							MakeTransition = true;
						}
						else {
							// Move within the obstacle SM
							NextState = ObstacleGeneratePathState;
							MakeTransition = true;
							break;
						}
				}
			}
			break;
		case ExitingObstacle:
			// Execute during function
			CurrentEvent = DuringExitingObstacleState(CurrentEvent);
			// Process events
			if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
			{
				switch (CurrentEvent.EventType)
				{
					case ES_TIMEOUT: //If event is event one
						if(CurrentEvent.EventParam == BACK_TO_COURSE_TIMER) {
							printf("Near Tape Line \r\n");
							ES_Event newEvent = {ToDriving, 0};
							PostMaster(newEvent);
						}
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

// During funciton for OrientState
static ES_Event DuringOrientState( ES_Event Event)
{
	// process ES_ENTRY & ES_EXIT events
	if ( Event.EventType == ES_ENTRY  ) {
		printf("Entered Orient State (Obstacle) \r\n");
			 
		// Find differnece in angles
		int deltaTheta;
		deltaTheta = 180 - myKart_Obstacle.KartTheta;//getDesiredTheta();

		printf("Vector Calculations Dist: %d  Desired Theta: %d \n\r", 180, getDesiredTheta());
		
		// Guarantee that the bot never rotates more than 180 degrees
		if(deltaTheta > 180) {
			deltaTheta = deltaTheta - 360;
		}
		if(deltaTheta < (-1)*180) {
			deltaTheta = deltaTheta + 360;
		}
		
		// Check direction of turn
		if(deltaTheta >= 0) {
			// Set to turn counter-clockwise, drive for length of drive timer
			HWREG(GPIO_PORTB_BASE + ALL_BITS) |= BIT3HI;
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
		}
		else {
			deltaTheta = abs(deltaTheta);
			// Set to turn clockwise, drive for length of drive timer
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~BIT3HI;
			HWREG(GPIO_PORTB_BASE + ALL_BITS) |=  BIT2HI;
		}	
		// Set PWM to (Half Speed)
		SetPWMDuty(HALF_SPEED_PORT,PORT_MOTOR);
		SetPWMDuty(HALF_SPEED_STARBOARD,STARBOARD_MOTOR);
		// Start timer for drive time
		if (deltaTheta == 0) {
				ES_Event NewEvent = {ES_TIMEOUT,OBS_TIMER};
				PostMaster(NewEvent);
		}
		else {
			ES_Timer_InitTimer(OBS_TIMER,ROTATION_TIME*deltaTheta/360);
		}
	}
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Turning State (Obstacle) \r\n");
		// No exit functionality 
	}
	else {
		// No during functionality
	}
	return( Event );  // Don't remap event
}

// During funciton for FindXState
static ES_Event DuringFindXState( ES_Event Event )
{
	// process ES_ENTRY & ES_EXIT events
	if ( Event.EventType == ES_ENTRY  ) {
		printf("Entered FindX State \r\n");
		// Set check timer 
		ES_Timer_InitTimer(OBS_TIMER,X_CheckTime);
		// Drive forward slowly
		printf("Drive Forward Slowly (1/8 speed) \r\n");
		// Drive forward slowly
		// Set left motor to drive forward
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT3HI;
		// Set right motor to drive forward
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
		// Set PWM to (1/4 Speed)
		SetPWMDuty(QUARTER_SPEED_PORT-10,PORT_MOTOR);
		SetPWMDuty(QUARTER_SPEED_STARBOARD-10,STARBOARD_MOTOR);
	}
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited FindX State \r\n");
		// No exit functionality 
	}
	else {
		// No during functionallity
	}
	return( Event );  // Don't remap event
}

// During funciton for ObstacleTurning1State
static ES_Event DuringObstacleTurning1State( ES_Event Event)
{
	// process ES_ENTRY & ES_EXIT events
	if ( Event.EventType == ES_ENTRY  ) {
		printf("Entered Turning State 1\r\n");
		// Find differnece in angles
		int deltaTheta;
		deltaTheta = OBSTACLE_ORIENTATION - myKart_Obstacle.KartTheta;//getDesiredTheta();
		printf("Vector Calculations Dist: %d  Desired Theta: %d \n\r", OBSTACLE_ORIENTATION, getDesiredTheta());
		// Guarantee that the bot never rotates more than 180 degrees
		if(deltaTheta > 180) {
		deltaTheta = deltaTheta - 360;
		}
		if(deltaTheta < (-1)*180) {
		deltaTheta = deltaTheta + 360;
		}

		// Check direction of turn
		if(deltaTheta >= 0) {
			// Set to turn counter-clockwise, drive for length of drive timer
			HWREG(GPIO_PORTB_BASE + ALL_BITS) |= BIT3HI;
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
		}
		else {
			deltaTheta = abs(deltaTheta);
			// Set to turn clockwise, drive for length of drive timer
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~BIT3HI;
			HWREG(GPIO_PORTB_BASE + ALL_BITS) |=  BIT2HI;
		}
		// Set PWM to (HalfSpeed)
		SetPWMDuty(HALF_SPEED_PORT,PORT_MOTOR);
		SetPWMDuty(HALF_SPEED_STARBOARD,STARBOARD_MOTOR);
		// Start timer for drive time
		if (deltaTheta == 0) {
			ES_Event NewEvent = {ES_TIMEOUT,OBS_TIMER};
			PostMaster(NewEvent);
		}
		else {
			ES_Timer_InitTimer(OBS_TIMER,ROTATION_TIME*deltaTheta/360 + (ROTATION_TIME*5/360));
		}
	}
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Turning State 1 \r\n");
		// No exit functionality 
	}
	else {
		// No during functionallity
	}
	return( Event );  // Don't remap event
}

// During funciton for FindYState
static ES_Event DuringFindYState( ES_Event Event)
{ 
	// process ES_ENTRY & ES_EXIT events
	if ( Event.EventType == ES_ENTRY  ) {
		printf("Entered FindY State \r\n");
		// set timer 
		ES_Timer_InitTimer(NITRO_TIMER,ONE_SEC);
		
		printf("Drive Forward Slowly \r\n");
		// Drive forward slowly
		// Set left motor to drive forward
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT3HI;
		// Set right motor to drive forward
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
		// Set PWM to (1/4 Speed)
		SetPWMDuty(HALF_SPEED_PORT,PORT_MOTOR);
		SetPWMDuty(HALF_SPEED_STARBOARD,STARBOARD_MOTOR);
	}
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited FindY State \r\n");
		// No exit functionality 
	}
	else {
		// No during functionality
	}
	return( Event );  // Don't remap event
}

// During funciton for ObstacleGeneratePathState
static ES_Event DuringObstacleGeneratePathState( ES_Event Event )
{
	// process ES_ENTRY & ES_EXIT events
	if ( Event.EventType == ES_ENTRY  ) {
		printf("Entered Generate Path State(Obstacle) \r\n");
		
		// Call calulate funciton with current point
		Calculate( ObstacleExit.X, ObstacleExit.Y );
	}
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Generate Path State (Obstacle) \r\n");
		// No exit functionality 
	}
	else {
		// No during functionality
	}
	return( Event );  // Don't remap event
}

// During funciton for ObstacleTurningState
static ES_Event DuringObstacleTurningState( ES_Event Event)
{
	// process ES_ENTRY & ES_EXIT events
	if ( Event.EventType == ES_ENTRY  ) {
		printf("Entered Turning State (Obstacle) \r\n");		 
		// Turn to next point	
		printf("Turn To Next Point \r\n");
		TurnTheta( );
	}
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Turning State (Obstacle) \r\n");
		// No exit functionality 
	}
	else {
		// No during functionality
	}
	return( Event );  // Don't remap event
}

// During funciton for DrivingForwardState
static ES_Event DuringObstacleDrivingForwardState( ES_Event Event)
{
	// process ES_ENTRY & ES_EXIT events
	if ( Event.EventType == ES_ENTRY  ) {
		printf("Entered Driving Forward State (Obstacle) \r\n");
		printf("Drive Forward \r\n");
		// Drive to next point
		DriveForward( );
	}
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Moving State (Obstacle) \r\n");
		// No exit functionality 
	}
	else {
		// No during functionality
	}
	return( Event );  // Don't remap event
}

// During funciton for ExitingObstacleState
static ES_Event DuringExitingObstacleState( ES_Event Event)
{
	// process ES_ENTRY & ES_EXIT events
	if ( Event.EventType == ES_ENTRY  ) {
		printf("Entered ExitingObstacle State \r\n");

		ES_Timer_InitTimer(BACK_TO_COURSE_TIMER,ONE_SEC);	
		
		// Drive forward
		// Set left motor to drive forward
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT3HI;
		// Set right motor to drive forward
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
		// Set PWM to (1/4 Speed)
		SetPWMDuty(QUARTER_SPEED_PORT-20,PORT_MOTOR);
		SetPWMDuty(QUARTER_SPEED_STARBOARD-20,STARBOARD_MOTOR);	
	}
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited ExitingObstacle State \r\n");
		// No exit functionality 
	}
	else {
		// No during functionallity
	}
	return( Event );  // Don't remap event
}
