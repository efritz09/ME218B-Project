/****************************************************************************
 Module
   GamePlay.c

 Revision			Revised by: 
	0.1.1				Alex
	0.1.2       Alex
	0.1.3				Alex

 Description
	Gameplay state machine that controls the driving, shooting, and obstacle
    crossing

 Edits:
	0.1.1 - Set up as template 
	0.1.2 - Changed to have running game state machine and pause state to remove "hack"
	0.1.3 - Modified pause state to implement last input to motors upon re-entry
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "Headers.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/
static ES_Event DuringRunningGameStateGP(ES_Event Event);
static ES_Event DuringPauseState(ES_Event Event);
static ES_Event DuringWaitForStartState(ES_Event Event);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static GamePlayState CurrentState;
static uint8_t lastPWM_Starboard;
static uint8_t lastPWM_Port;
static int dirStar;
static int dirPort;

/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
     StartGamePlay

 Parameters
     Event_t CurrentEvent

 Returns
     None

 Description
     Sets initial state to WaitForStartState
****************************************************************************/
void StartGamePlay ( ES_Event CurrentEvent )
{
	// Local variable to get debugger to display the value of CurrentEvent
	volatile ES_Event LocalEvent = CurrentEvent;
	CurrentState = WaitForStartState;
	// Call the entry function for the ENTRY_STATE
	RunGamePlay(CurrentEvent);
}

/****************************************************************************
 Function
    RunGamePlay

 Parameters
   unsigned char: the event to process

 Returns
   unsigned char: an event to return

 Description
   run function for game play state machine
****************************************************************************/
ES_Event RunGamePlay( ES_Event CurrentEvent )
{
	bool MakeTransition = false;/* are we making a state transition? */
	GamePlayState NextState = CurrentState;
	ES_Event EntryEventKind = { ES_ENTRY, 0 };// default to normal entry to new state
	ES_Event ReturnEvent = CurrentEvent; // assume we are not consuming event

	switch ( CurrentState )
	{
		case RunningGameStateGP :
			// Execute During function for driving state
			CurrentEvent = DuringRunningGameStateGP(CurrentEvent);
			// Process any events
			if ( CurrentEvent.EventType != ES_NO_EVENT )
			{
				 switch (CurrentEvent.EventType)
				 {
					 case CautionFlagDropped :
						 printf("Caution Flag Dropped \r\n");
						 // Move to pause state
						 NextState = PauseState;
						 MakeTransition = true;
						 break;
					 case GameOver :
						 printf("Game Over \r\n");
						 // Restart to wait for start state
						 NextState = WaitForStartState;
						 MakeTransition = true;
						 break;
					 case EmergencyStop :
						 printf("Emergency Stop \r\n");
						 // Move to pause state
						 NextState = PauseState;
						 MakeTransition = true;
						 break;
				 }
			}
			break;
		case PauseState :
			// Execute During function for pause state
			CurrentEvent = DuringPauseState(CurrentEvent);
			// Process any events
			if ( CurrentEvent.EventType != ES_NO_EVENT )
			{
				switch (CurrentEvent.EventType)
				{
					case FlagDropped :
						printf("Caution Over \r\n");
						// Change entry event to with history
						EntryEventKind.EventType = ES_ENTRY_HISTORY;
						// Move to running game state
						NextState = RunningGameStateGP;
						MakeTransition = true;
						break;
					case GameOver :
					  printf("Game Over \r\n");
						// Restart to wait for start state
						NextState = WaitForStartState;
						MakeTransition = true;
						break;
				}
			}
			break;		 
		case WaitForStartState :
			// Execute During function for pause state
			CurrentEvent = DuringWaitForStartState(CurrentEvent);
			// Process any events
			if ( CurrentEvent.EventType != ES_NO_EVENT )
			{
				switch (CurrentEvent.EventType)
				{
					case FlagDropped :
						printf("Flag Dropped \r\n");
						// Move to running game state
						NextState = RunningGameStateGP;
						MakeTransition = true;
						break;
				}
			}
		break;
	}
	//   If we are making a state transition
	if (MakeTransition == true)
	{
		 //   Execute exit function for current state
		 CurrentEvent.EventType = ES_EXIT;
		 RunGamePlay(CurrentEvent);

		 CurrentState = NextState; //Modify state variable

		 //   Execute entry function for new state
		 CurrentEvent.EventType = ES_ENTRY;
		 RunGamePlay(EntryEventKind);
	 }
	 return(ReturnEvent);
}

/****************************************************************************
 Function
     QueryGamePlay

 Parameters
     None

 Returns
     The current state of the state machine

 Description
     Returns the current state of the state machine
****************************************************************************/
GamePlayState QueryGamePlay ( void )
{
	return(CurrentState);
}

/***************************************************************************
 private functions
 ***************************************************************************/

// During Function for RunningGameStateGP
static ES_Event DuringRunningGameStateGP(ES_Event Event)
{
	// Local variable to get debugger to display the value of CurrentEvent
	volatile ES_Event NewEvent = Event;

	// Process ES_ENTRY & ES_EXIT events
	if (Event.EventType == ES_ENTRY ) {
		printf("Entered Running Game State \r\n");
		// Start Running Game state machine and Drive service in parallel
		StartRunningGame(Event);
		StartDrive(Event);
	}
	else if (Event.EventType == ES_EXIT) {
		printf("Exited Running Game State \r\n");
		// On exit, give the lower levels a chance to clean up first
		NewEvent = RunRunningGame(Event);
		RunDrive(Event);
	}
	else {
		// Do the 'during' function for this state
		// Run Running Game state machine, Drive service, and Ball Shooter service in parallel
		NewEvent = RunRunningGame(Event);
		RunDrive(Event);
		RunBallShooter(Event);
	}
	return(NewEvent);
}

// During Function for PauseState
static ES_Event DuringPauseState(ES_Event Event)
{
	static bool TimerStates[numTimers];
	
	// Local variable to get debugger to display the value of CurrentEvent
	volatile ES_Event NewEvent = Event;

	// Process ES_ENTRY & ES_EXIT events
	if (Event.EventType == ES_ENTRY) {
		printf("Entered Pause State \r\n");
		
		// Save motor states
		lastPWM_Starboard = GetLastPWM(0);
		lastPWM_Port = GetLastPWM(1);
		dirPort = HWREG(GPIO_PORTB_BASE + ALL_BITS) & BIT3HI;
		dirStar = HWREG(GPIO_PORTB_BASE + ALL_BITS) & BIT2HI;
		
		printf("Kill Motors \r\n");
		// Kill motors
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
		SetPWMDuty(0,STARBOARD_MOTOR);
		SetPWMDuty(0,PORT_MOTOR);
		
		// Loop through all timers
		for(int i = 0;i < numTimers;i++) {
			// Track active timers 
			if(ES_Timer_isActive(i)){
				TimerStates[i] = true;
			}
			// Pause all timers
			if(i!=DRS_TIMER){
				ES_Timer_StopTimer(i);
			}
		}
	}
	else if (Event.EventType == ES_EXIT) {
		printf("Exited Pause State \r\n");
		// Restart timers that were active before
		for(int i = 0;i < numTimers;i++) {
			printf("%d ", TimerStates[i]);
			if(TimerStates[i]) {
				ES_Timer_StartTimer(i);
			}
		}
		// Reset motors to previous state
		if(!dirPort) {
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT3HI);
		}
		else {
			HWREG(GPIO_PORTB_BASE + ALL_BITS) |= (BIT3HI);
		}
		
		if(!dirStar) {
			HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI);
		}
		else {
			HWREG(GPIO_PORTB_BASE + ALL_BITS) |= (BIT2HI);
		}
		
		SetPWMDuty(lastPWM_Starboard,STARBOARD_MOTOR);
		SetPWMDuty(lastPWM_Port,PORT_MOTOR);
		
		printf("\r\n");
		printf("%d \r\n", QueryRunningGame());
	}
	else {
		// No during functionality
	}
	return(NewEvent);
}

// During Function for WaitForStartState
static ES_Event DuringWaitForStartState(ES_Event Event)
{
	// Local variable to get debugger to display the value of CurrentEvent
	volatile ES_Event NewEvent = Event;

	// Process ES_ENTRY & ES_EXIT events
	if (Event.EventType == ES_ENTRY) {
		printf("Entered Wait For Start State \r\n");
		printf("Kill Motors \r\n");
		// Kill motors
		HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT2HI | BIT3HI);
		SetPWMDuty(0,STARBOARD_MOTOR);
		SetPWMDuty(0,PORT_MOTOR);
	}
	else if (Event.EventType == ES_EXIT) {
		printf("Exited Wait For Start State \r\n");
		// No exit functionality
	}
	else {
		RunBallShooter(Event);
	}
	return(NewEvent);
}
