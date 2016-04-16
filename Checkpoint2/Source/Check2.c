/****************************************************************************
 Module
   Check2.c

 Revision			Revised by: 
	0.1.1					Alex						2/17/15

 Description
  PWM service to initialize the Tiva's hardware PWM output to drive our
	two DC motors, and two servo motors.
	 
 Edits:
	0.1.1 - Created for purpose of passing the driving checkpoint

****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/

// TIVA headers
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_pwm.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "bitdefs.h"

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

// ES_framework headers
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Port.h"
#include "ES_Types.h"
#include "ES_Events.h"

// Module headers
#include "PWM.h"
#include "Check2.h"

/*----------------------------- Module Defines ----------------------------*/
// 40Mhz clock assumes 40 ticks per microS, we will use SysClk/32 for PWM
#define ALL_BITS 							(0xff<<2)
#define BitsPerNibble 				4
#define PWMTicksPerMicroS 		40/32

// Symbolic defines for time
#define ONE_MS 								1000			// In ticks?
#define TWO_MS 								2000			// In ticks?

// Symbolic defines for PWM periods to be used
// DC motor time constant should be measured!!!
#define PeriodInMicroS 				1000			// DC motors: 1kHz for now (add caps to 18200!)
#define ServoPeriod 					20000 		// Servo motors: standard 20 ms

// Motor times
#define NINETY_DEGREE_TIME (ONE_SEC)
#define STRAIGHTAWAY_TIME (ONE_SEC*3)



/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behaviour of this service
*/
static void InitInputCaptureWT1(void);
void EncoderInterrputResponse(void);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static int numStraights = 0;
static C2State CurrentState;

/*------------------------------ Module Code ------------------------------*/


/****************************************************************************
 Function
	InitCheck2

 Parameters
	none

 Returns
	True/False

 Description
****************************************************************************/
bool InitCheck2 ( uint8_t Priority )
{
	InitPWM();
	
	CurrentState = IdleState;
	
	MyPriority = Priority;
	return true;
}

/****************************************************************************
 Function
	RunCheck2

 Description
  Runs a pseudo-state machine to test our DC motors
****************************************************************************/
ES_Event RunCheck2 ( ES_Event ThisEvent )
{
	ES_Event ReturnEvent = {ES_NO_EVENT};
	
	switch(CurrentState) {
		case IdleState :
			// Start on a straightaway if button is pressed
			if(ThisEvent.EventType == DBButtonUp) {
				ES_Event NewEvent = {goStraight, 0};
				PostCheck2(NewEvent);
			}
			// Drive striaght and start timer
			if(ThisEvent.EventType == On) {
				// Move state
				CurrentState = StraightState;
				
				// Drive forward half-speed
				printf("Drive forward half-speed\r\n");
				// Set left motor to drive forward
				HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT5HI;
				// Set right motor to drive forward
				HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT4HI;
				// Set PWM to (HalfSpeed)
				SetPWMDuty(HALF_SPEED_PORT,PORT_MOTOR);
				SetPWMDuty(HALF_SPEED_PORT,STARBOARD_MOTOR);
				
				//Start straightaway timer
				ES_Timer_InitTimer(STRAIGHTAWAY_TIMER, STRAIGHTAWAY_TIME);
			}
			break;
		case StraightState :
			// Kill motors and restart if button is pressed
			if(ThisEvent.EventType == Off) {
				// Murder the motors
				HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT4HI | BIT5HI);
				SetPWMDuty(0,STARBOARD_MOTOR);
				SetPWMDuty(0,PORT_MOTOR);
				numStraights = 0;
				
				// Move state
				CurrentState = IdleState;
			}
			// If still running laps, turn once motor timer expires
			if(numStraights >= 4) {
				// Murder the motors
				HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT4HI | BIT5HI);
				SetPWMDuty(0,STARBOARD_MOTOR);
				SetPWMDuty(0,PORT_MOTOR);
				numStraights = 0;
				
				// Move state
				CurrentState = IdleState;
			}
			else {
				if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == STRAIGHTAWAY_TIMER) {
					// Murder the motors
					HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT4HI | BIT5HI);
					SetPWMDuty(0,STARBOARD_MOTOR);
					SetPWMDuty(0,PORT_MOTOR);
					
					// Increment number of straights completed
					numStraights++;
					// Post turnLeft Event
					ES_Event NewEvent = {turnLeft, 0};
					PostCheck2(NewEvent);
				}
				if(ThisEvent.EventType == turnLeft) {
					// Move state
					CurrentState = TurnState;
					
					// Rotate counter-clockwise by 90 degrees 
					printf("Rotate counter-clockwise 90 degrees\r\n");
					// Set left motor to drive in reverse
					HWREG(GPIO_PORTB_BASE + ALL_BITS) |= BIT5HI;
					// Set right motor to drive forward
					HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT4HI);
					// Set PWM to (50%)
					SetPWMDuty(100 - HALF_SPEED_PORT,PORT_MOTOR);
					SetPWMDuty(HALF_SPEED_STARBOARD,STARBOARD_MOTOR);
					
					// Set timer for a 90 degree turn
					ES_Timer_InitTimer(TURN_TIMER,NINETY_DEGREE_TIME);
				}
			}
			break;
		case TurnState :
			// Kill motors and restart if button is pressed
			if(ThisEvent.EventType == DBButtonUp) {
				// Murder the motors
				HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT4HI | BIT5HI);
				SetPWMDuty(0,STARBOARD_MOTOR);
				SetPWMDuty(0,PORT_MOTOR);
				numStraights = 0;
				
				// Move state
				CurrentState = IdleState;
			}
			// If still running laps, turn once motor timer expires
			if(numStraights >= 4) {
				// Murder the motors
				HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT4HI | BIT5HI);
				SetPWMDuty(0,STARBOARD_MOTOR);
				SetPWMDuty(0,PORT_MOTOR);
				numStraights = 0;
				
				// Move State
				CurrentState = IdleState;
			}
			else {
				if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == TURN_TIMER) {
					// Murder the motors
					HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT4HI | BIT5HI);
					SetPWMDuty(0,STARBOARD_MOTOR);
					SetPWMDuty(0,PORT_MOTOR);
					
					// Post goStraight Event
					ES_Event NewEvent = {goStraight, 0};
					PostCheck2(NewEvent);
				}
				if(ThisEvent.EventType == goStraight) {
					// Move state
					CurrentState = StraightState;
					
					// Drive forward half-speed
					printf("Drive forward half-speed\r\n");
					// Set left motor to drive forward
					HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT5HI;
					// Set right motor to drive forward
					HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~ BIT4HI;
					// Set PWM to (HalfSpeed)
					SetPWMDuty(HALF_SPEED_PORT,PORT_MOTOR);
					SetPWMDuty(HALF_SPEED_PORT,STARBOARD_MOTOR);
					
					//Start straightaway timer
					ES_Timer_InitTimer(STRAIGHTAWAY_TIMER, STRAIGHTAWAY_TIME);
				}
			}
			break;
	}
	return ReturnEvent;	
}

/****************************************************************************
 Function
	PostCheck2

 Description
    Posts an event to this service's queue
****************************************************************************/
bool PostCheck2 ( ES_Event ThisEvent )
{
	
  return ES_PostToService( MyPriority, ThisEvent);
}

void EncoderInterrputResponse(void) {
	HWREG(WTIMER1_BASE + TIMER_O_ICR) = TIMER_ICR_CAECINT;
	ES_Event CableEvent = {Encoder, 0};
	PostCheck2(CableEvent);
	static int ticks = 0;
	ticks++;
	//printf("%d",ticks);
}


static void InitInputCaptureWT1( void )
	{
		/*
	Steps to initialize Tiva Wide Timer Input Capture
	1.	Enable the clock to wide timer 1 using RCGCWTIMER_R1 register
	2.	Enable the clock to the appropriate GPIO module RCGCGPIO
	3.	Disable wtimer 1 timer A to configure with TAEN in TIMER_O_CTL register
	4.	Set timer to 32bit wide mode with TIMER_CFG_16_BIT in TIMER_O_CTL register
	5.	Set timer to use full 32bit count by setting TIMER_O_TAILR to 0xffffffff
	6.	Set timer A to capture mode for edge time and up-counting (Clear TAMMS and Set TACMR,TACDIR,TAMR_CAP in TIMERTAMR)
	7.	Set event to rising edge by clearing TAEVENT_M in TIMER_O_CTL (Rising edge = 00)
	8.	Select the alternate function for the Timer pins (AFSEL)
	9.	Configure the PMCn fields in the GPIOPCTL register to assign the WTIMER pins (WT1CCP0)
	10.	Enable appropriate pint on GPIO port for digital I/O (GPIODIR)
	11.	Set appropriate pins on GPIO as inputs (GPIODEN)
	12. Locally enable interrupts for the capture interrupt mask (CAEIM in TIMERIMR)
	13. Enable WTIMER1 Timer A interrupt vector in NVIC (96 = Bit 0 in NVIC_EN3) (Tiva p.104 for vector table)
	14. Ensure interrupts are enabled globally (__enable_irq())
	14.	Enable WTIMER1 Timer A with TAEN in TIMER_O_CTL register
	
	WTIMER1 Timer A - Tiva PC6
	*/
		
		
  // Enable the clock to the timer (Wide Timer 1)
  HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R1;
		
	// Enable the clock to Port C
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R2;
  
  // Disable timer A & B before configuring
  HWREG(WTIMER1_BASE + TIMER_O_CTL) &= ~(TIMER_CTL_TAEN | TIMER_CTL_TBEN);
	
  // Set up in 32bit wide (individual, not concatenated) mode
  HWREG(WTIMER1_BASE + TIMER_O_CFG) = TIMER_CFG_16_BIT;

  // Use the full 32 bit count, so initialize the Interval Load
  // register to 0xffff.ffff
  HWREG(WTIMER1_BASE + TIMER_O_TAILR) = 0xffffffff;
	HWREG(WTIMER1_BASE + TIMER_O_TBILR) = 0xffffffff;
	
  // set up timer A in capture mode (TAAMS = 0), 
  // for edge time (TACMR = 1) and up-counting (TACDIR = 1)
  HWREG(WTIMER1_BASE + TIMER_O_TAMR) = 
      (HWREG(WTIMER1_BASE + TIMER_O_TAMR) & ~TIMER_TAMR_TAAMS) | 
        (TIMER_TAMR_TACDIR | TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP);

	HWREG(WTIMER1_BASE + TIMER_O_TBMR) = 
      (HWREG(WTIMER1_BASE + TIMER_O_TBMR) & ~TIMER_TBMR_TBAMS) | 
        (TIMER_TBMR_TBCDIR | TIMER_TBMR_TBCMR | TIMER_TBMR_TBMR_CAP);
				
  // To set the event to rising edge, we need to modify the TAEVENT bits 
  // in GPTMCTL. Rising edge = 00, so we clear the TAEVENT bits
  HWREG(WTIMER1_BASE + TIMER_O_CTL) &= ~(TIMER_CTL_TAEVENT_M | TIMER_CTL_TAEVENT_BOTH);

  // Set up the alternate function for Port C bit 6 (WT1CCP0)
  HWREG(GPIO_PORTC_BASE + GPIO_O_AFSEL) |= (BIT6HI | BIT7HI);

	// Set mux position on GPIOPCTL to select WT1CCP0 alternate function on C6
	// Mux value = 7 offset mask to clear nibble 6
	HWREG(GPIO_PORTC_BASE + GPIO_O_PCTL) = 
    (HWREG(GPIO_PORTC_BASE + GPIO_O_PCTL) & ~0x0f000000) + (7 << ( 6 * BitsPerNibble)) + 
																													 (7 << ( 7 * BitsPerNibble));
			
  // Enable pin 6 & 7 on Port C for digital I/O
  HWREG(GPIO_PORTC_BASE + GPIO_O_DEN) |= (BIT6HI | BIT7HI);
	
  // Make pin 6 & 7 on Port C into an input
  HWREG(GPIO_PORTC_BASE + GPIO_O_DIR) &= (BIT6LO & BIT7LO);

  // Enable a local capture interrupt
  HWREG(WTIMER1_BASE + TIMER_O_IMR) |= (TIMER_IMR_CAEIM | TIMER_IMR_CBEIM);

  // Enable Timer A Wide Timer 1 interrupt in the NVIC
  // NVIC number 96 (EN3 bit 0) Tiva p.104
  HWREG(NVIC_EN3) |= (BIT0HI | BIT1HI);

  // Make sure interrupts are enabled globally
  __enable_irq();

  // Enable Timer
  HWREG(WTIMER1_BASE + TIMER_O_CTL) |= (TIMER_CTL_TAEN | TIMER_CTL_TASTALL | 
																				TIMER_CTL_TBEN | TIMER_CTL_TBSTALL);
	
	// Print to console if successful initialization
	printf("Wide Timer 1 A&B interrupt initialized\n\r");
}
