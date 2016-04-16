/****************************************************************************
 Module
   Button.c

 Description
   This module implements a two-state state machine for debouncing timing
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for the framework and this service
*/

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "Check2.h"
#include "Button.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"

/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC (976)
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)

#define ALL_BITS (0xff << 2)
#define ButtonPin (GPIO_PIN_4)
#define OutputPin (GPIO_PIN_3)
#define DEBOUNCE_MS (50)

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
static void InitButtonHardware(void);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static uint8_t LastButtonState;
static ButtonState CurrentState;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitializeButton

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, samples button port pin and uses it to initialize LastButtonState variable
		 Sets current state to DEBOUNCING
		 Starts debounce timer
 Notes

 Author
     Alex Yee 10-27-14
****************************************************************************/
bool InitButton ( uint8_t Priority )
{
	printf("Init Button");
	printf("\r\n");
	ES_Event ThisEvent;

	MyPriority = Priority;
	
	// Initialize port line to monitor button
	InitButtonHardware();
	
	// Set current state
	LastButtonState = HWREG(GPIO_PORTF_BASE + (GPIO_O_DATA + ALL_BITS)) & ButtonPin;
	printf("%d\r\n", LastButtonState);
	CurrentState = DEBOUNCING;
	
	// Start debounce timer (service timer 0)
	ES_Timer_InitTimer(DebounceTimerNumber, DEBOUNCE_MS);
	
  // post the initial transition event
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
     PostButton

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     Alex Yee 10-29-14
****************************************************************************/
bool PostButton( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunButtonDebounceSM

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   Implements a 2-state state machine for debouncing timing
 Notes
   
 Author
   Alex Yee 10-29-14
****************************************************************************/
ES_Event RunButton( ES_Event ThisEvent ) {
  
	ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
	
	// Button state machine
	switch(CurrentState) {
		case DEBOUNCING:
			// Ready to sample once debounce timer runs out
			if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == DebounceTimerNumber) {
				CurrentState = READY2SAMPLE;
			}
			break;
		case READY2SAMPLE:
			ES_Event NewEvent;
			// If sampling and button event occurs (either up or down), post to stepService
			if(ThisEvent.EventType == ButtonUp) {
				ES_Timer_InitTimer(DebounceTimerNumber, DEBOUNCE_MS);
				CurrentState = DEBOUNCING;
				NewEvent.EventType = DBButtonUp;
				PostCheck2(NewEvent);
			}
			if(ThisEvent.EventType == ButtonDown) {
				ES_Timer_InitTimer(DebounceTimerNumber, DEBOUNCE_MS);
				CurrentState = DEBOUNCING;
				NewEvent.EventType = DBButtonDown;
				PostCheck2(NewEvent);
			}
			break;
		}
	return ReturnEvent;
}

// Getter for LastButtonState
uint8_t getLastButtonState(void) {
	return LastButtonState;
}

// Setter for LastButtonState
void setLastButtonState(uint8_t last) {
	LastButtonState = last;
	return;
}

/***************************************************************************
 private functions
 ***************************************************************************/

static void InitButtonHardware(void) {
	volatile uint32_t Dummy;
	
	//Enable the Port F
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R5;
	
	// Kill a few cycles to let the peripheral clock get going
	Dummy = HWREG(SYSCTL_RCGCGPIO);

  // Enable pin 4 for digital I/O
	HWREG(GPIO_PORTF_BASE + GPIO_O_DEN) |= (GPIO_PIN_4);

  // Make pin 4 on port F into an input
	HWREG(GPIO_PORTF_BASE + GPIO_O_DIR) &= ~(GPIO_PIN_4);
}
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

