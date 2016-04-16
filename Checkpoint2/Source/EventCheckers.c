/****************************************************************************
 Module
   EventCheckers.c

 Revision
   1.0.1 

 Description
   Keystroke events to simulate the Command Generator's commands

 Notes
   
****************************************************************************/

// this will pull in the symbolic definitions for events, which we will want
// to post in response to detecting events
#include "ES_Configure.h"
// this will get us the structure definition for events, which we will need
// in order to post events in response to detecting events
#include "ES_Events.h"
// if you want to use distribution lists then you need those function 
// definitions too.
#include "ES_PostList.h"
// This include will pull in all of the headers from the service modules
// providing the prototypes for all of the post functions
#include "ES_ServiceHeaders.h"
// this test harness for the framework references the serial routines that
// are defined in ES_Port.c
#include "ES_Port.h"
// include our own prototypes to insure consistency between header & 
// actual functionsdefinition
#include "EventCheckers.h"
#include "Button.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"

#define ALL_BITS (0xff << 2)


/****************************************************************************
 Function
   Check4Keystroke
 Parameters
   None
 Returns
   bool: true if a new key was detected & posted
 Description
   checks to see if a new key from the keyboard is detected and, if so, 
   posts the appropriate command to the SPI service
 Notes
   The functions that actually check the serial hardware for characters
   and retrieve them are assumed to be in ES_Port.c
   Since we always retrieve the keystroke when we detect it, thus clearing the
   hardware flag that indicates that a new key is ready this event checker 
   will only generate events on the arrival of new characters, even though we
   do not internally keep track of the last keystroke that we retrieved.
 Author
   J. Edward Carryer, 08/06/13, 13:48
****************************************************************************/
bool Check4Keystroke(void)
{
  if ( IsNewKeyReady() ) // new key waiting?
  {
    ES_Event ThisEvent;
    ThisEvent.EventType = ES_NEW_KEY;
    ThisEvent.EventParam = GetNewKey();
    // test distribution list functionality by sending the 'L' key out via
    // a distribution list.
    if ( ThisEvent.EventParam == '1'){
	  ES_Event newEvent = {On, 1};
      PostCheck2( newEvent );
	  printf("POST - On \r\n");
    }
	else if ( ThisEvent.EventParam == '2'){
	 ES_Event newEvent = {Off, 2};
     PostCheck2( newEvent );
	 printf("POST - Offt \r\n");
    }
	else if ( ThisEvent.EventParam == '3'){
	 ES_Event newEvent = {shit, 2};
     PostCheck2( newEvent );
	 printf("POST - Offt \r\n");
    }
	
	else{   // otherwise post to Service 0 for processing
		printf("POST - Unknown key \r\n");
    }
    return true;
  }
  return false;
}

bool CheckButtonEvents( void ) {
	ES_Event ThisEvent;
	bool returnVal = false;
	uint8_t CurrentButtonState = HWREG(GPIO_PORTF_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT4HI;
	
	// Check to see if the button state has changed
	if(CurrentButtonState != getLastButtonState()) {
		returnVal = true;
		// Button down
		if(CurrentButtonState == 0) {
			ThisEvent.EventType = ButtonUp;
			PostButton(ThisEvent);
		}
		// Button up
		else {
			ThisEvent.EventType = ButtonDown;
			PostButton(ThisEvent);
		}
	}
	setLastButtonState(CurrentButtonState);
	return returnVal;
}

