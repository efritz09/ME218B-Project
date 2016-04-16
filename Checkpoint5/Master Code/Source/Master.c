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
#include "Headers.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
static ES_Event DuringMaster( ES_Event Event);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

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
