/****************************************************************************
 Module
   Obstacle.c

 Revision			Revised by: 
	0.1.1				Alex
	0.2.1				Alex
	0.3.1				Alex

 Description
	Obstacle crossing state machine that controls traversing the obstacle
	
	could add swinging motion when finding line to make sure we get there

 Edits:
	0.1.1 - Set up as template 
	0.2.1 - Update obstacle state machine for tape finding with two analog tape sensors
	0.3.1 - Update to use timers to cross the obstacle
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "Headers.h"

/*----------------------------- Module Defines ----------------------------*/
// define constants for this machine

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
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/
static ES_Event DuringOrientState( ES_Event Event); 
static ES_Event DuringFindXState( ES_Event Event); 
static ES_Event DuringTurningState( ES_Event Event);
static ES_Event DuringFindYState( ES_Event Event);
static ES_Event DuringDrivingToTeeterPointState( ES_Event Event);
static ES_Event DuringPauseAtTeeterState( ES_Event Event);
static ES_Event DuringExitingObstacleState( ES_Event Event);


/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static ObstacleState CurrentState;
KART_t myKart_Obstacle;
static bool rerun = false;


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
	
	 myKart_Obstacle = QueryMyKart();
	
	
   switch ( CurrentState )
   {
		 case Orient :
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
								else {
									ES_Timer_InitTimer(CHECK_TIMER,X_CheckTime);
								}
						 break;
						 }
					 }				                
				break;
		 case FindX :
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
       case Turning :
         // Execute during function
         CurrentEvent = DuringTurningState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
          switch (CurrentEvent.EventType)
            {
               case ES_TIMEOUT:
								// Move forward if the bot is at around the correct X value
								if(CurrentEvent.EventParam == OBS_TIMER) {
										printf("At 270 \r\n");
									//if(rerun) {
										//NextState = FindX;
										//rerun = false;
									//}else {
										NextState = FindY;
									//}
										MakeTransition = true;
								}
							}
						}
           break;
        case FindY :
         // Execute during function
         CurrentEvent = DuringFindYState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case ES_TIMEOUT: //If event is event one
								 //printf("Kart y position: %d\r\n",myKart_Obstacle.KartY);
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
									 
								 } else if(CurrentEvent.EventParam == OBS_TIMER) {
									 
											printf("At Beginging of obstacle \r\n");
											
											if(myKart_Obstacle.KartY < (y1)) {
												printf("At Correct Y \r\n");
												NextState = ExitingObstacle;
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
		/*case DrivingToTeeterPoint :
         // Execute during function
         CurrentEvent = DuringDrivingToTeeterPointState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case ES_TIMEOUT: //If event is event one
								printf("On the Obstacle \r\n");
                  if(myKart_Obstacle.KartY >= Y_teeter) {
										printf("At Correct Y \r\n");
										NextState = PauseAtTeeter;
										MakeTransition = true;
									}
									else {
										// Re-intitialize timer
										ES_Timer_InitTimer(OBS_TIMER,Y_CheckTime);
									} //mark that we are taking a transition
                  break;
            }
         }
         break;
			case PauseAtTeeter :
         // Execute during function
         CurrentEvent = DuringPauseAtTeeterState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
          switch (CurrentEvent.EventType)
            {
               case ES_TIMEOUT:
								// continue down the ramp
										printf("PauseEnded \r\n");
										NextState = ExitingObstacle;
										MakeTransition = true;
							}
						}
           break;*/
			  case ExitingObstacle :
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
	CurrentState = Orient;
	
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
static ES_Event DuringOrientState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY  ) {
			printf("Entered Orient State (Obstacle) \r\n");
				 
			// Find differnece in angles
			int deltaTheta;
			deltaTheta = 180 - myKart_Obstacle.KartTheta;//getDesiredTheta();
		
			printf("Vector Calculations Dist: %d  Desired Theta: %d \n\r", 180, getDesiredTheta());
			
			//garuntee that the bot never rotates more than 180 degrees
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
				}else {
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
			
			//goes forward until you hit a specific x value
		}
	else if ( Event.EventType == ES_EXIT) {
        // No exit functionality 
		printf("Exited FindX State \r\n");
	}
	else {
		// No during functionallity
	}
    return( Event );  // Don't remap event
}

static ES_Event DuringTurningState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY  ) {
		printf("Entered Turning State \r\n");
			
		// Find differnece in angles
		int deltaTheta;
		deltaTheta = OBSTACLE_ORIENTATION - myKart_Obstacle.KartTheta;//getDesiredTheta();
	
	printf("Vector Calculations Dist: %d  Desired Theta: %d \n\r", OBSTACLE_ORIENTATION, getDesiredTheta());
	
	//garuntee that the bot never rotates more than 180 degrees
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
				}else {
					ES_Timer_InitTimer(OBS_TIMER,ROTATION_TIME*deltaTheta/360 + (ROTATION_TIME*5/360));
				}
	
	}
	else if ( Event.EventType == ES_EXIT) {
    // No exit functionality 
		printf("Exited Turning State \r\n");
	}
	else {
		// No during functionallity
    }
    return( Event );  // Don't remap event
}

static ES_Event DuringFindYState( ES_Event Event)
{ 
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY  ) {
			//ES_Timer_InitTimer(GIVE_UP_TIMER,ONE_SEC*10);
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
    // No exit functionality 
		printf("Exited FindY State \r\n");
    }
	else {
		// No during functionality
    }
    return( Event );  // Don't remap event
}

static ES_Event DuringDrivingToTeeterPointState( ES_Event Event)
{
	// process ES_ENTRY & ES_EXIT events
	if ( Event.EventType == ES_ENTRY  ) {
			printf("Entered DrivingToTeeterPoint State \r\n");
			// set timer 
			ES_Timer_InitTimer(OBS_TIMER,ONE_SEC*1.5);
			
			printf("Drive Forward rapidly \r\n");
			// Drive forward slowly
			// Set left motor to drive forward
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT3HI;
			// Set right motor to drive forward
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
			// Set PWM to (FUCKIN FAST Speed)
			SetPWMDuty(FULL_SPEED_PORT,PORT_MOTOR);
			SetPWMDuty(FULL_SPEED_STARBOARD,STARBOARD_MOTOR);
	}
	else if ( Event.EventType == ES_EXIT) {
				printf("Exited DrivingToTeeterPoint State \r\n");
    }
	else {
		// No during functionality
    }
    return( Event );  // Don't remap event
}

static ES_Event DuringPauseAtTeeterState( ES_Event Event )
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY) {
		// Start a waiting timer to allow for the obstacle to tilt before decending
			ES_Timer_InitTimer(OBS_TIMER,ONE_SEC/2);
		}
	else if ( Event.EventType == ES_EXIT) {
        // No exit functionality 
		printf("Exited PauseAtTeeter State \r\n");
	}
	else {
		// No during functionallity
	}
    return( Event );  // Don't remap event
}


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
    // No exit functionality 
		printf("Exited ExitingObstacle State \r\n");
		
	}
	else {
		// No during functionallity
    }
    return( Event );  // Don't remap event
}
