/****************************************************************************
 Module
   EncoderService.c

	Revision			Revised by: 
	0.1.1					Alex						2/5/15
	0.1.2					Alex						2/22/15

 Description
   Encoder service to set up input capture needed for both encoders and a 20 ms timer
   
   Need to add something to throw speed change events
	 
	Edits:
	0.1.1 - Created for lab7
	0.2.1 - Updated to control both motors on one 20 ms timer
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include <cmath>
#include "PWM.h"
#include "EncoderService.h"

#include "ES_Port.h"
#include "termio.h"
#include "inc/hw_memmap.h" 
#include "inc/hw_types.h" 
#include "inc/hw_gpio.h" 
#include "inc/hw_sysctl.h"

#include "inc/hw_timer.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_nvic.h"
#include "bitdefs.h"

#include <stdint.h>
#include "inc/hw_pwm.h"


/*----------------------------- Module Defines ----------------------------*/
#define ONE_SEC 976
#define TicksPerMS 40000
#define fullCycle (50)
#define MSperMin (1000*60)
#define ALL_BITS (0xff<<2)
#define RPMtimer (1)
#define RPMtime (ONE_SEC)
#define maxPeriod (35000)
#define minPeriod (14500)

#define ONE_SEC 976
#define BitsPerNibble 4

// 40,000 ticks per mS assumes a 40Mhz clock, we will use SysClk/32 for PWM
#define PWMTicksPerMS 4000/32
// Set 200 Hz frequency so 5mS period
#define PeriodInMS 2

#define controlLawTime 20

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
void InitInputCapturePeriod( void );
void InputCaptureResponse( void );
void ControlLaw( void );
void setDuty( int duty );
void InitPeriodicInt( void );

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static uint32_t periodA;
static uint32_t periodB;
static uint32_t lastCaptureA = 0;
static uint32_t lastCaptureB = 0;
static float refSpeedA = 0;
static float refSpeedB = 0;
static float sumErrorA = 0;
static float errorA = 0;
static float sumErrorB = 0;
static float errorB = 0;

static int requestedDutyA = 0;
static int requestedDutyB = 0;

static float pGain = 1.5;
static float iGain = 0.1;




/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitEncoderService

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, initializes TIVA pins, sets up init capture,
		 starts RPM timer
****************************************************************************/
bool InitEncoderService ( uint8_t Priority )
{
  ES_Event ThisEvent;

  MyPriority = Priority;
	
	// Setup init capture
	InitInputCapturePeriod( );
	
	// Setup periodic timer
	InitPeriodicInt( );
	
	// Start RPM timer
	ES_Timer_InitTimer(RPMtimer, RPMtime);

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
     PostEncoderService

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
****************************************************************************/
bool PostEncoderService( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunEncoderService

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   Determines period after every full revolution, changes LED lighting based on period,
	 time various calculations
****************************************************************************/
ES_Event RunEncoderService( ES_Event ThisEvent )
{

  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

	if(ThisEvent.EventType == SpeedChange_PORT) {
		refSpeedA = ThisEvent.EventParam;
		sumErrorA = 0;
	}
	if(ThisEvent.EventType == SpeedChange_STARBOARD) {
		refSpeedB = ThisEvent.EventParam;
		sumErrorB = 0;
	}
	
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/
void InitInputCapturePeriod( void ){

  // Enable the clock to the timer (Wide Timer 1)
  HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R1;
	// enable the clock to Port C
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R2;
  
  // Disable timer A and B before configuring
  HWREG(WTIMER1_BASE + TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
  HWREG(WTIMER1_BASE + TIMER_O_CTL) &= ~TIMER_CTL_TBEN;
	
  // Set up in 32bit wide (individual, not concatenated) mode
  HWREG(WTIMER1_BASE + TIMER_O_CFG) = TIMER_CFG_16_BIT;

  // Use the full 32 bit count, so initialize the Interval Load
  // register to 0xffff.ffff
  HWREG(WTIMER1_BASE + TIMER_O_TAILR) = 0xffffffff;

  // set up timer A in capture mode (TAMR = , TAAMS = 0), 
  // for edge time (TACMR = 1) and up-counting (TACDIR = 1)
  HWREG(WTIMER1_BASE + TIMER_O_TAMR) = 
      (HWREG(WTIMER1_BASE + TIMER_O_TAMR) & ~TIMER_TAMR_TAAMS) | 
        (TIMER_TAMR_TACDIR | TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP);
		
  // set up timer B in capture mode (TBMR = 3, TBAMS = 0), 
  // for edge time (TBCMR = 1) and up-counting (TBCDIR = 1)
  HWREG(WTIMER1_BASE + TIMER_O_TBMR) = 
      (HWREG(WTIMER1_BASE + TIMER_O_TBMR) & ~TIMER_TBMR_TBAMS) | 
        (TIMER_TBMR_TBCDIR | TIMER_TBMR_TBCMR | TIMER_TBMR_TBMR_CAP);

  // To set the event to rising edge, we need to modify the TAEVENT bits 
  // in GPTMCTL. Rising edge = 00, so we clear the TAEVENT bits
  HWREG(WTIMER1_BASE + TIMER_O_CTL) &= ~TIMER_CTL_TAEVENT_M;
  
  // To set the event to rising edge, we need to modify the TAEVENT bits 
  // in GPTMCTL. Rising edge = 00, so we clear the TBEVENT bits
  HWREG(WTIMER1_BASE + TIMER_O_CTL) &= ~TIMER_CTL_TBEVENT_M;

  // Set up the alternate function for Port C bits 6 and 7 (WT1CCP0 and WT1CCP1)
  HWREG(GPIO_PORTC_BASE + GPIO_O_AFSEL) |= (BIT6HI | BIT7HI);

  // Map bit 6's alternate function to WT1CCP0
  // 7 is the mux value to select WT1CCP0, 24 to shift it over to the
  // right nibble for bit 6 (4 bits/nibble * 6 bits)
  // Map bit 7's alternate function to WT1CCP1
  // 7 is the mux value to select WT1CCP0, 28 to shift it over to the
  // right nibble for bit 7 (4 bits/nibble * 7 bits)
  HWREG(GPIO_PORTC_BASE + GPIO_O_PCTL) = 
  (HWREG(GPIO_PORTC_BASE + GPIO_O_PCTL) & 0xfffff00f) + (7 << 24) + (7 << 28);

  // Enable pins 6 and 7 on Port C for digital I/O
  HWREG(GPIO_PORTC_BASE + GPIO_O_DEN) |= (BIT6HI | BIT7HI);
	
  // Make pins 6 and 7 on Port C into an input
  HWREG(GPIO_PORTC_BASE + GPIO_O_DIR) &= (BIT6LO & BIT7LO);

  // Enable a local capture interrupt
  HWREG(WTIMER1_BASE + TIMER_O_IMR) |= TIMER_IMR_CAEIM;
  HWREG(WTIMER1_BASE + TIMER_O_IMR) |= TIMER_IMR_CBEIM;

  // Enable the Timers A and B in Wide Timer 1 interrupt in the NVIC
  // they are interrupt numbers 96 and 97 (EN3 bit 0 and 1)
  HWREG(NVIC_EN3) |= BIT0HI;
  HWREG(NVIC_EN3) |= BIT1HI;

  // Make sure interrupts are enabled globally
  __enable_irq();


  // Enable Timers
  HWREG(WTIMER1_BASE + TIMER_O_CTL) |= (TIMER_CTL_TAEN | TIMER_CTL_TBEN |TIMER_CTL_TASTALL | TIMER_CTL_TBSTALL);
}

// Port interrupt capture response is associated with Wide Timer 1A
void PortEncoderResponse( void ){
  
  
	// Start by clearing the source of the interrupt, the input capture event
    HWREG(WTIMER1_BASE + TIMER_O_ICR) = TIMER_ICR_CAECINT;
		
	uint32_t ThisCaptureA;
	// Now grab the captured value and calculate the period
	ThisCaptureA = HWREG(WTIMER0_BASE+TIMER_O_TAR);
	periodA = ThisCaptureA - lastCaptureA;
	
	// Update LastCapture to prepare for the next edge  
	lastCaptureA = ThisCaptureA;
	
}

// Starboard interrupt capture response is associated with Wide Timer 1B
void StarboardEncoderResponse( void ){
  
	// Start by clearing the source of the interrupt, the input capture event
    HWREG(WTIMER1_BASE + TIMER_O_ICR) = TIMER_ICR_CBECINT;
		
	uint32_t ThisCaptureB;
	// Now grab the captured value and calculate the period
	ThisCaptureB = HWREG(WTIMER0_BASE+TIMER_O_TAR);
	periodB = ThisCaptureB - lastCaptureB;
	
	// Update LastCapture to prepare for the next edge  
	lastCaptureB = ThisCaptureB;
	
}

void ControlLaw( void ) {
	// Calculate current RPM values
	float RPMA = (MSperMin)/((periodA*fullCycle)/TicksPerMS);
	float RPMB = (MSperMin)/((periodB*fullCycle)/TicksPerMS);
	
	// Clear interrupt
	HWREG(WTIMER0_BASE + TIMER_O_ICR) = TIMER_ICR_TBTOCINT;
	
	// Implement control law for both encoders
	errorA = refSpeedA - RPMA;
	sumErrorA += errorA;
	requestedDutyA = pGain*errorA + iGain*sumErrorA;
	
	errorB = refSpeedB - RPMB;
	sumErrorB += errorB;
	requestedDutyB = pGain*errorB + iGain*sumErrorB;
	
	// Anti-windup A
	if(requestedDutyA > 100) {
		requestedDutyA = 100;
		sumErrorA -= errorA;
	}
	else if(requestedDutyA < 0) {
		requestedDutyA = 0;
		sumErrorA -= errorA;
	}
	
	// Anti-windup B
	if(requestedDutyB > 100) {
		requestedDutyB = 100;
		sumErrorB -= errorB;
	}
	else if(requestedDutyB < 0) {
		requestedDutyB = 0;
		sumErrorB -= errorB;
	}
	
	SetPWMDuty(requestedDutyA, PORT_MOTOR);
	SetPWMDuty(requestedDutyB, STARBOARD_MOTOR);
}

// Initialize periodic timer to go off every 20 ms
void InitPeriodicInt( void ){

 volatile uint32_t Dummy; // use volatile to avoid over-optimization
  
  // Start by enabling the clock to the timer (Wide Timer 0)
  HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R0;
  
  // Kill a few cycles to let the clock get going
  Dummy = HWREG(SYSCTL_RCGCGPIO);
  
  // Make sure that timer (Timer B) is disabled before configuring
  HWREG(WTIMER0_BASE + TIMER_O_CTL) &= ~TIMER_CTL_TBEN;
  
  // Set it up in 32bit wide (individual, not concatenated) mode
  HWREG(WTIMER0_BASE + TIMER_O_CFG) = TIMER_CFG_16_BIT;
  
  // Set up timer B in periodic mode so that it repeats the time-outs
  HWREG(WTIMER0_BASE + TIMER_O_TBMR) = 
     (HWREG(WTIMER0_BASE + TIMER_O_TBMR)& ~TIMER_TBMR_TBMR_M)| 
     TIMER_TBMR_TBMR_PERIOD;

  // Set timeout to 20mS
  HWREG(WTIMER0_BASE + TIMER_O_TBILR) = TicksPerMS * controlLawTime;

  // Enable a local timeout interrupt
  HWREG(WTIMER0_BASE + TIMER_O_IMR) |= TIMER_IMR_TBTOIM;

  // Enable the Timer B in Wide Timer 0 interrupt in the NVIC
  // it is interrupt number 95 so apppears in EN2 at bit 31
  HWREG(NVIC_EN2) = BIT31HI;

  // Make sure interrupts are enabled globally
  __enable_irq();
  
  // Enable timer
  HWREG(WTIMER0_BASE + TIMER_O_CTL) |= (TIMER_CTL_TBEN | TIMER_CTL_TBSTALL);
}

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
