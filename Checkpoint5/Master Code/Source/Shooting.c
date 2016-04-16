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
#include "Headers.h"

/*----------------------------- Module Defines ----------------------------*/
// define constants for this machine

#define LOAD_TIME ONE_SEC
#define FIRE_TIME ONE_SEC
#define BEACON_LOW (30000)
#define BEACON_HIGH (33000)

#define X_CheckTime 100

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/
static ES_Event DuringOrientState( ES_Event Event );
static ES_Event DuringFindYState( ES_Event Event );
static ES_Event DuringShootingTurningState( ES_Event Event );
static ES_Event DuringFindXState( ES_Event Event );
static ES_Event DuringFindBeaconState( ES_Event Event );
static ES_Event DuringFireState( ES_Event Event );
static ES_Event DuringReturnToTapeState( ES_Event Event );
void BeaconCaptureResponse( void );
bool FindBeacon (void);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static ShootingState CurrentState;
static uint32_t LastCapture;
static uint32_t BeaconPeriod;
static bool BeaconLeftTurn = false;
//static bool ValidLastPeriod;

POINT_t currentPoint_Shooting;
KART_t myKart_Shooting;

POINT_t ShootingPoint_Shooting = {SP_X,SP_Y};

static bool SearchBeacon = false; //do we search for beacon? T or F

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
	
		myKart_Shooting = QueryMyKart( );
		currentPoint_Shooting.X = myKart_Shooting.KartX;
		currentPoint_Shooting.Y = myKart_Shooting.KartY;
	
   //printf("Event posted to shooting \r\n");
   switch ( CurrentState )
   {
		  case OrientState :
         // Execute during function
         CurrentEvent = DuringOrientState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
          switch (CurrentEvent.EventType)
            {
               case ES_TIMEOUT:
								// Move forward if the bot is at around the correct X value
								if(CurrentEvent.EventParam == CHECK_TIMER) {
										printf("At 90 \r\n");
										NextState = FindYState;
										MakeTransition = true;
									 // Murder the motors
										HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
										SetPWMDuty(0,STARBOARD_MOTOR);
										SetPWMDuty(0,PORT_MOTOR);
								}
							}
						}
         break;
			case FindYState :
         // Execute during function
         CurrentEvent = DuringFindYState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case ES_TIMEOUT:
									// If at shooting point, move on
									if(CurrentEvent.EventParam == CHECK_TIMER) {
										if(currentPoint_Shooting.Y >= ShootingPoint_Shooting.Y) {
											
											// Murder the motors
											HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
											SetPWMDuty(0,STARBOARD_MOTOR);
											SetPWMDuty(0,PORT_MOTOR);
											
											
											NextState = ShootingTurningState;//Decide what the next state will be
											MakeTransition = true; //mark that we are taking a transition
										}
										else {
											ES_Timer_InitTimer(CHECK_TIMER,X_CheckTime);
										}
									}
									break;
            }
         }
         break;
        case ShootingTurningState :
         // Execute during function
         CurrentEvent = DuringShootingTurningState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
          switch (CurrentEvent.EventType)
            {
               case ES_TIMEOUT:
								// Move forward if the bot is at around the correct X value
								if(CurrentEvent.EventParam == CHECK_TIMER) {
										printf("At 180 \r\n");
										NextState = FindXState;
										MakeTransition = true;
								}
							}
						}
         break;
				case FindXState :
         // Execute during function
         CurrentEvent = DuringFindXState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case ES_TIMEOUT:
									// If at shooting point, move on
									if(CurrentEvent.EventParam == CHECK_TIMER) {
										if(currentPoint_Shooting.X >= ShootingPoint_Shooting.X) {
											
											// Murder the motors
											HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
											SetPWMDuty(0,STARBOARD_MOTOR);
											SetPWMDuty(0,PORT_MOTOR);
											
											
											NextState = FindBeaconState;//Decide what the next state will be
											MakeTransition = true; //mark that we are taking a transition
										}
										else {
											ES_Timer_InitTimer(CHECK_TIMER,X_CheckTime);
										}
									}
									break;
            }
         }
         break;
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
										 SearchBeacon = false;
									 }else {
										 // Set right motor to drive in reverse
												HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~BIT2HI;
												// Set left motor to drive forward
												HWREG(GPIO_PORTB_BASE + ALL_BITS) |= BIT3HI;
										 SearchBeacon = true; 
									 }
									 ES_Timer_InitTimer(CHECK_TIMER,ONE_SEC);
								 } else if (CurrentEvent.EventParam == GIVE_UP_TIMER) {
									 printf("Give up, peace out\r\n");
									 NextState = ReturnToTapeState;
									 MakeTransition = true;
									 // Murder the motors
									HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
									SetPWMDuty(0,STARBOARD_MOTOR);
									SetPWMDuty(0,PORT_MOTOR);
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
									NextState = ReturnToTapeState;//Decide what the next state will be
									MakeTransition = true; //mark that we are taking a transition
								}
                break;
            }
         }
         break;
				case ReturnToTapeState :
         // Execute during function
         CurrentEvent = DuringReturnToTapeState(CurrentEvent);
         // Process events
         if ( CurrentEvent.EventType != ES_NO_EVENT )        //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case ES_TIMEOUT :
								 if(CurrentEvent.EventParam == BACK_TO_COURSE_TIMER) {
										printf("At Near Tape Point \r\n");
									  
									  // Murder the motors
										HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
										SetPWMDuty(0,STARBOARD_MOTOR);
										SetPWMDuty(0,PORT_MOTOR);
									 
									  // Transistion to driving SM
									  ES_Event newEvent = {ToDriving, 0};
										PostMaster(newEvent);
								}
							}
              break;
           }
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
	
	CurrentState = OrientState;

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
static ES_Event DuringOrientState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY  ) {
			printf("Entered Orient State (Shooting) \r\n");
				 
			// Find differnece in angles
			int deltaTheta;
			deltaTheta = 90 - myKart_Shooting.KartTheta;//getDesiredTheta();
		
			printf("Vector Calculations Dist: %d  Desired Theta: %d \n\r", 90, myKart_Shooting.KartTheta);
			
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
					ES_Event NewEvent = {ES_TIMEOUT,CHECK_TIMER};
					PostMaster(NewEvent);
					printf("theta = 0\r\n");
				}else {
					ES_Timer_InitTimer(CHECK_TIMER,ROTATION_TIME*deltaTheta/360);
					printf("theta != 0\r\n");
				}
				printf("is timer active %d\r\n",ES_Timer_isActive(CHECK_TIMER));
    }
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Orient State (Shooting) \r\n");
        // No exit functionality 
    }
	else {
		// No during functionality
    }
    return( Event );  // Don't remap event
}

static ES_Event DuringFindYState( ES_Event Event )
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY  ) {
			printf("Entered Find Y State \r\n");
					
			printf("%d %d %d \n\r", myKart_Shooting.KartX, myKart_Shooting.KartY, myKart_Shooting.KartTheta);
			printf("Drive Forward \r\n");
			// Drive slowly
			// Set Motors forward, drive for length of drive timer
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT3HI;
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
			// Set PWM to (quarter Speed)
			SetPWMDuty(QUARTER_SPEED_PORT-10,PORT_MOTOR);
			SetPWMDuty(QUARTER_SPEED_STARBOARD-10,STARBOARD_MOTOR);
			
			// Start timer to check Y
			ES_Timer_InitTimer(CHECK_TIMER,X_CheckTime);
		}

	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Find Y State (Shooting) \r\n");
        // No exit functionality 
    }
	else {
		// No during functionallity	
    }
    return( Event );  // Don't remap event
}

static ES_Event DuringShootingTurningState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY  ) {
			printf("Entered Turning State (Shooting) \r\n");
				 
			// Find differnece in angles
		int deltaTheta;
		deltaTheta = 180 - myKart_Shooting.KartTheta;//getDesiredTheta();
	
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
			
			// Set PWM to (HalfSpeed)
			SetPWMDuty(HALF_SPEED_PORT,PORT_MOTOR);
			SetPWMDuty(HALF_SPEED_STARBOARD,STARBOARD_MOTOR);
			// Start timer for drive time
			if (deltaTheta == 0) {
					ES_Event NewEvent = {ES_TIMEOUT,CHECK_TIMER};
					PostMaster(NewEvent);
			}else {
				ES_Timer_InitTimer(CHECK_TIMER,ROTATION_TIME*deltaTheta/360 + 50);
			}
    }
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Turning State (Shooting) \r\n");
        // No exit functionality 
    }
	else {
		// No during functionality
    }
    return( Event );  // Don't remap event
}

static ES_Event DuringFindXState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY  ) {
			printf("Entered Find X State \r\n");
			printf("Drive Forward \r\n");
			// Drive slowly
			// Set Motors forward, drive for length of drive timer
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT3HI;
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT2HI;
			// Set PWM to (1/8 Speed)
			SetPWMDuty(QUARTER_SPEED_PORT-10,PORT_MOTOR);
			SetPWMDuty(QUARTER_SPEED_STARBOARD-10,STARBOARD_MOTOR);
			
			// Start timer to check X
			ES_Timer_InitTimer(CHECK_TIMER,X_CheckTime);
    }
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Find X State (Shooting) \r\n");
        // No exit functionality 
    }
	else {
		// No during functionality
    }
    return( Event );  // Don't remap event
}

static ES_Event DuringFindBeaconState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
		ES_Timer_InitTimer(GIVE_UP_TIMER,ONE_SEC*10);
    if ( Event.EventType == ES_ENTRY  ) {
			printf("Entered Find Beacon State \r\n");
			
			printf("Rotate Slowly \r\n");
			// Counter-clockwise rotation
			// Set left motor to drive in reverse
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~BIT3HI;
			// Set right motor to drive forward
			HWREG(GPIO_PORTB_BASE + ALL_BITS) |= BIT2HI;
			// Set PWM to 1/4 speed
			SetPWMDuty(QUARTER_SPEED_PORT,PORT_MOTOR);
			SetPWMDuty(QUARTER_SPEED_STARBOARD,STARBOARD_MOTOR);
			
			ES_Timer_InitTimer(CHECK_TIMER,ONE_SEC/2);
			
			printf("Load Ball \r\n");
			// Get it in
			// Open gate to let one ball out
			printf("Open Gate (let one ball out) \r\n");
			ES_Timer_InitTimer(LOAD_BALL_TIMER,LOAD_TIME);
			
			SearchBeacon = true;
    }
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Find Beacon State \r\n");
        // No exit functionality 
		SearchBeacon = false;
    }
	else {
			// No during functionallity
    }
    return( Event );  // Don't remap event
}

static ES_Event DuringFireState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
		ES_Timer_StopTimer(GIVE_UP_TIMER);
    if ( Event.EventType == ES_ENTRY  ) {
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

static ES_Event DuringReturnToTapeState( ES_Event Event)
{
    // process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY  ) {
		printf("Entered Return To Tape State \r\n");
		printf("Forward for two seconds \r\n");
		
    // Go forward
		// Set left motor to drive reverse
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~BIT3HI;
		// Set right motor to drive reverse
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~BIT2HI;
		// Set PWM to (1/2 Speed)
		SetPWMDuty(HALF_SPEED_PORT,PORT_MOTOR);
		SetPWMDuty(HALF_SPEED_STARBOARD,STARBOARD_MOTOR);			
			
		// Start reverse timer
		ES_Timer_InitTimer(BACK_TO_COURSE_TIMER,ONE_SEC/2);
		
    }
	else if ( Event.EventType == ES_EXIT) {
		printf("Exited Return To Tape State \r\n");
        // No exit functionality
    }
	else {
		// No during functionality
    }
    return( Event );  // Don't remap event
}

bool InitBeaconCaptureResponse (void)
{
	// Start by enabling the clock to the timer (Wide Timer 0)
  HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R0;
	// Enable the clock to Port C
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R2;
	// Since we added this Port C clock init, we can immediately start
  // into configuring the timer, no need for further delay
  
  // Make sure that Wide Timer 0 Timer A is disabled before configuring
  HWREG(WTIMER0_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
  
	// Set Wide Timer 0 Timer A in 32bit wide (individual, not concatenated) mode
	// the constant name derives from the 16/32 bit timer, but this is a 32/64
	// bit timer so we are setting the 32bit mode
  HWREG(WTIMER0_BASE+TIMER_O_CFG) = TIMER_CFG_16_BIT;

	// We want to use the full 32 bit count, so initialize the Interval Load
	// register to 0xffff.ffff (this is its default value)
  HWREG(WTIMER0_BASE+TIMER_O_TAILR) = 0xffffffff;

	// Vet up Wide Timer 0 Timer A in capture mode (TAMR=3, TAAMS = 0), 
	// for edge time (TACMR = 1) and up-counting (TACDIR = 1)
  HWREG(WTIMER0_BASE+TIMER_O_TAMR) = 
      (HWREG(WTIMER0_BASE+TIMER_O_TAMR) & ~TIMER_TAMR_TAAMS) | 
        (TIMER_TAMR_TACDIR | TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP);

	// To set the event to rising edge, we need to modify the TAEVENT bits 
	// in GPTMCTL. Rising edge = 00, so we clear the TAEVENT bits
  HWREG(WTIMER0_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEVENT_M;

	// Now Set up PortC Pin4 to do the capture (clock was enabled earlier)
	// start by setting the alternate function for Port C bit 4 (WT0CCP0)
	HWREG(GPIO_PORTC_BASE+GPIO_O_AFSEL) |= BIT4HI;

	// Then, map bit 4's alternate function to WT0CCP0
	// 7 is the mux value to select WT0CCP0, 16 to shift it over to the
	// right nibble for bit 4 (4 bits/nibble * 4 bits)
	HWREG(GPIO_PORTC_BASE+GPIO_O_PCTL) = 
    (HWREG(GPIO_PORTC_BASE+GPIO_O_PCTL) & 0xfff0ffff) + (7<<16);

	// Enable pin 4 on Port C as digital I/O
	HWREG(GPIO_PORTC_BASE+GPIO_O_DEN) |= BIT4HI;
	
	// make pin 4 on Port C into an input
	HWREG(GPIO_PORTC_BASE+GPIO_O_DIR) &= BIT4LO;

	// back to the timer to enable a local capture interrupt
  HWREG(WTIMER0_BASE+TIMER_O_IMR) |= TIMER_IMR_CAEIM;

	// enable the Timer A in Wide Timer 0 interrupt in the NVIC
	// it is interrupt number 94 so appears in EN2 at bit 30
  HWREG(NVIC_EN2) |= BIT30HI;

	// make sure interrupts are enabled globally
  __enable_irq();

	// now kick the timer off by enabling it and enabling the timer to
	// stall while stopped by the debugger
  HWREG(WTIMER0_BASE+TIMER_O_CTL) |= (TIMER_CTL_TAEN | TIMER_CTL_TASTALL);
	
	printf("IRSensor Initialized\n\r");
	return true;
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
	
	if(SearchBeacon) {
		if( (BeaconPeriod > BEACON_LOW) && (BeaconPeriod < BEACON_HIGH)){
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
