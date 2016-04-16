/****************************************************************************
 Module
   Master.c

 Revision			Revised by: 
	0.1.1				Alex
	0.1.2				Alex

 Description
	Master state machine that contains all other state machines for the Kart

 Edits:
	0.1.1 - Set up as template to test hierarchical state machine mechanics
	0.1.2 - Include printouts in all modules to follow states with keystrokes
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
#include "RunningGame.h"
#include "Master.h"
#include "Driving.h"
#include "Shooting.h"
#include "Obstacle.h"
#include "BallShooter.h"

/*----------------------------- Module Defines ----------------------------*/
#define ONE_SEC 976

/*---------------------------- Module Functions ---------------------------*/
static ES_Event DuringMaster( ES_Event Event);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;
bool InitBeaconCaptureResponse(void);

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitMasterSM

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority,  and starts
     the top level state machine
****************************************************************************/
bool InitMaster ( uint8_t Priority )
{
  ES_Event ThisEvent;

  MyPriority = Priority;  // save our priority
	
  // Initialize PWM and non-PWM motor pins
  InitPWM( );
	InitBallShooter();
	InitBeaconCaptureResponse();
	
  ThisEvent.EventType = ES_ENTRY;
  
  // Start the Master State machine
  StartMaster( ThisEvent );

  return true;
}

/****************************************************************************
 Function
     PostMaster

 Parameters
     ES_Event ThisEvent , the event to post to the queue

 Returns
     bool false if the post operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
****************************************************************************/
bool PostMaster( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunMaster

 Parameters
   ES_Event: the event to process

 Returns
   ES_Event: an event to return

 Description
   The run function for the top level state machine 

    Consider adding global clear
****************************************************************************/
ES_Event RunMaster( ES_Event CurrentEvent )
{
    // Start by passing events to the lower level machines
    CurrentEvent = DuringMaster(CurrentEvent);
    
    // We might want to add a global clear later
    /*
    if ( CurrentEvent.EventType == EV_CLEAR)
    {
        // First pass exit messages to the lower level machines
        CurrentEvent.EventType = ES_EXIT;
        RunMasterSM(CurrentEvent);
        // Now pass entry messages, since this is a self transition
        CurrentEvent.EventType = ES_ENTRY;
        RunMasterSM(CurrentEvent);
    }
    */
	
	//printf("Event posted to master \r\n");
   
    // In the absence of an error the top level state machine should
    // always return ES_NO_EVENT
    CurrentEvent.EventType = ES_NO_EVENT;
    return(CurrentEvent);
}
/****************************************************************************
 Function
     StartMaster

 Parameters
     ES_Event CurrentEvent

 Returns
     nothing

 Description
     Does any required initialization for this state machine
****************************************************************************/
void StartMaster ( ES_Event CurrentEvent )
{
    // local variable to get debugger to display the value of CurrentEvent
    volatile ES_Event LocalEvent = CurrentEvent;
    // there is only 1 state to the top level machine so no need for a state
    // variable. All we need to do is to let the Run function init the lower level
    // state machines
   
    // use LocalEvent to keep the compiler from complaining about unused var
    RunMaster(LocalEvent);
    return;
}


/***************************************************************************
 private functions
 ***************************************************************************/

static ES_Event DuringMaster( ES_Event Event )
{
    // Process ES_ENTRY & ES_EXIT events
    if ( Event.EventType == ES_ENTRY) {
        // Implement any entry actions required for this state machine
		
		printf("Entered Master \r\n");
        
        // Start any lower level machines that run in this state
        // Call start functions for gameplay SM and SPI SM
        
        StartDRS(Event);
        StartGamePlay(Event);
        
    }
    else if ( Event.EventType == ES_EXIT) {
        // On exit, give the lower levels a chance to clean up first
        // Call run function for gameplay SM and SPI SM
		
		printf("Exited Master \r\n");
        
        RunDRS(Event);
        RunGamePlay(Event);
				
    }
    else {
        // Do the 'during' function for this state
        
        // Run any lower level state machine
        // since we have concurrent state machines below this level
        // we can not alter the Event that is passed
        // Call gameplay SM and SPI SM run functions
        
        RunDRS(Event);
        RunGamePlay(Event);
				
    }
    return(Event);
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

