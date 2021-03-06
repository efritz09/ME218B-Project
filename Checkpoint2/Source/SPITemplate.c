/****************************************************************************
 Module
	DRSTemplate.c

 Revision			Revised by: 
	0.1.1				Denny
	0.1.2				Denny
	0.2.1				Denny
	0.2.2				Denny

 Description
	SPI state machine service to communicate with the DrEd Reckoning system 
	for the project (TIVA SPI Module 0). Handles all comunication between the
	Tiva and DRS. Commands are intiated from a timer, reads are interrupt driven.

 Edits:
	0.1.1 - Set up as template to test the RS SPI interface. Right now we are
	assuming we are KART1, so query only switches between GameState and KART1.
	This will need to be updated to provide KART variability later.
	0.1.2 - KARTs converted to structures. Right now there is no history in this
	module to track changes or post changes, add this later or have parallel Game
	state machine do the checking (Or just handle with event checkers). We still
	need to implement hardware initializations to read which kart we are.
	0.2.1 - Changed InitDRS to only call StartDRS, and StartDRS will initializa and
	call RunDRS so we can easily implement the Master SM which will call StartDRS.
	0.2.2 - Changed the SPI system so the SS line is now pulled up by the DRS board.
	This was done by selecting Pin A3 as Open Drain with GPOI_O_ODR (See StartDRS)

****************************************************************************/
// If we are debugging and setting our own Game/KART states
//#define TEST

/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include <stdint.h>
#include <stdbool.h>
#include <cmath>

// ES_framework headers
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Port.h"
#include "ES_Types.h"
#include "ES_Events.h"

// TIVA headers
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_pwm.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "inc/hw_ssi.h"
#include "inc/hw_nvic.h"
#include "driverlib/ssi.h"
#include "bitdefs.h"

// Module headers
#include "SPITemplate.h"
#include "Master.h"


/*----------------------------- Module Defines ----------------------------*/
#define BitsPerNibble 			4
#define TicksPerMS 					40000
#define ALL_BITS 						(0xff<<2)

// Symbolic defines for query options to DRS
#define QUERY_GAME_STATE 		0x3F
#define QUERY_KART1 				0xC3
#define QUERY_KART2 				0x5A
#define QUERY_KART3 				0x7E
#define INVALID_READ 				0xFF

// Symbolic defines for DRS responses
#define OBSTICAL_COMPLETE 	0x40
#define SHOT_COMPLETE				0x80
#define WAIT_FOR_START			0x00
#define FLAG_DROPPED				0x01
#define CAUTION_FLAG				0x02
#define RACE_OVER						0x03
#define GAME_STATE_MASK			0x18
#define LAPS_REMAINING_MASK	0x07

// Define timeouts for command transfers
#define DRS_COMMAND_TIMEOUT 	10		// Tick count for SendCommand timeout (10 ms)
#define DRS_COMMAND_DELAY		1000		// Tick count for 2ms delay between commands (3 ms)


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behaviour of this service
*/
static bool DRSSendQuery ( void );
static uint8_t DRSQuerySelect( void );
static bool DRSSaveData ( void );
static ES_Event DuringDRS_Ready( ES_Event Event);
static ES_Event DuringDRS_Transfer( ES_Event Event);
static ES_Event DuringDRS_Wait( ES_Event Event);


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static DRSState_t CurrentState; 		// Create state for DRS data transfer

static uint8_t CurrentQuery;				// Keep track of the current query
static uint8_t LastQuery;						// Save the last query in case of transfer failure
static bool EOTResponseFlag;				// Track if EOT interrupt occured

static uint8_t MY_KART;							// Save what our KART number is
static KART_t Kart1;								// Structure for KART1 (All KART information)
static KART_t Kart2;								// Structure for KART2 (All KART information)
static KART_t Kart3;								// Structure for KART3 (All KART information)

static uint8_t NewDRSRead[8] =  {0x00, 0xFF, 0x00, 0xD9, 0x03, 0x03, 0x00, 0x00};				// Array to save 8 byte response from DRS
//static uint8_t NewRSRead[8] = {0x00, 0xFF, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00};

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
	InitDRS (Delete when we bring this SM into the Master)

 Parameters
	uint8_t : the priorty of this service

 Returns
	bool, false if error in initialization, true otherwise

 Description
	Saves away the priority, initializes TIVA pins A2-5 as SPI to communicate
	with the DrEd Reckoning System
		A2: SSI module 0 clock (SCK)
		A3: SSI module 0 frame signal (SS)
		A4: SSI module 0 receive (SDI)
		A5: SSI module 0 transmit (SDO)
****************************************************************************/
bool InitDRS  ( uint8_t Priority )
{
	ES_Event ThisEvent;
	MyPriority = Priority;
	
	// Call StartDRS (This will be done by the master SM when we integrate)
	ThisEvent.EventType = ES_ENTRY;
	StartDRS( ThisEvent );
	
	// Return true for now (we won't need this function when HSM is implemented)
	return true;
}

/****************************************************************************
 Function
	StartDRS 

 Parameters
	ES_Event ThisEvent ,the event to post to the queue

 Returns
	none

 Description
	Initializes TIVA pins A2-5 as SPI to communicate
	with the DrEd Reckoning System
		A2: SSI module 0 clock (SCK)
		A3: SSI module 0 frame signal (SS)
		A4: SSI module 0 receive (SDI)
		A5: SSI module 0 transmit (SDO)
****************************************************************************/
void StartDRS( ES_Event CurrentEvent )
{
	/**************** Initialize SPI to communicate with DRS *****************/
	
	// Enable clock to the GPIO port (A)
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R0;
	
	// Enable clock to the SSI Module 0
	HWREG(SYSCTL_RCGCSSI) |= SYSCTL_RCGCSSI_R0;
	
	// Wait for GPIO port A to be ready
	while ((HWREG(SYSCTL_PRSSI) & SYSCTL_PRSSI_R0 ) != SYSCTL_PRSSI_R0 )
		;
	
	// Program pins A2-A5 as alternate functions on the GPIO to use SSI (Set High) 
	HWREG(GPIO_PORTA_BASE + GPIO_O_AFSEL) |= (BIT2HI | BIT3HI | BIT4HI | BIT5HI);
	
	// Select the SSI alternate functions on pins A2-A5
	// Set mux position on GPIOPCTL to select the use of the pins (2 for SSI)
	HWREG(GPIO_PORTA_BASE + GPIO_O_PCTL) |= 
    (HWREG(GPIO_PORTA_BASE + GPIO_O_PCTL) & ~0x00ffff00) + (2 << ( 2 * BitsPerNibble)) +
      (2 << ( 3 * BitsPerNibble)) + (2 << ( 4 * BitsPerNibble)) +
      (2 << ( 5 * BitsPerNibble));
	
	// Program port lines for pins A2-A5 as digital I/O
	HWREG(GPIO_PORTA_BASE + GPIO_O_DEN) |= (BIT2HI | BIT3HI | BIT4HI | BIT5HI);
	
	// Program required data directions on the port lines (2/3/5 Output, 4 Input)
	HWREG(GPIO_PORTA_BASE + GPIO_O_DIR) |= (BIT2HI | BIT3HI | BIT5HI);
	HWREG(GPIO_PORTA_BASE + GPIO_O_DIR) &= ~BIT4HI;
	
	// Program pullup resistor on clock line (A2, A4)
	HWREG(GPIO_PORTA_BASE + GPIO_O_PUR) |= (BIT2HI | BIT4HI);
	
	// Changes: GPIO_O_ODR
	// SS (Pin A3) now configured as open drain, see TIVA Data Sheet page 676
	HWREG(GPIO_PORTA_BASE + GPIO_O_ODR) |= (BIT3HI);
	
	// Wait for the SSI0 Module 0 to be ready
	while ((HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_BSY ) == SSI_SR_BSY )
		;
	
	// Make sure SSI Module 0 is disabled before programming mode bits
	HWREG(SSI0_BASE + SSI_O_CR1) &= ~SSI_CR1_SSE;
	
	// Select master mode (MS) & TXRIS interrupt indicating end of transmission (EOT)
	HWREG(SSI0_BASE + SSI_O_CR1) &= ~SSI_CR1_MS; 
	HWREG(SSI0_BASE + SSI_O_CR1) |= SSI_CR1_EOT;

	// Configure the SSI clock source to the system clock
	HWREG(SSI0_BASE + SSI_O_CC) &= ~(BIT0HI | BIT1HI | BIT2HI | BIT3HI); //SET ALL BITS LOW (0-3, pg 984)
	
	// Configure the clock pre-scaler
	HWREG(SSI0_BASE + SSI_O_CPSR) |= BIT1HI;
	HWREG(SSI0_BASE + SSI_O_CPSR) &= ~(BIT2HI | BIT3HI | BIT4HI | BIT5HI | BIT6HI | BIT7HI); //ENSURE THE REST ARE 0
	
	// Configure the clock rate (SCR), phase (SPH) & polarity (SPO), mode (FRF), and data size (DSS)
	HWREG(SSI0_BASE + SSI_O_CR0) |=  0x0A00; 												// Set SCR to 10
	HWREG(SSI0_BASE + SSI_O_CR0) |=  (SSI_CR0_SPH | SSI_CR0_SPO);		// Set SPH and SPO to 1
	HWREG(SSI0_BASE + SSI_O_CR0) |= ~(BIT4HI | BIT5HI); 						// Set frame mode (pg 969)
	HWREG(SSI0_BASE + SSI_O_CR0) |= SSI_CR0_DSS_8; 									// Set data size to 8-bit
	
	// Locally enable interrupts on TXRIS
	// (bits 0-3, pg 977)E ARE ALL LOW (bits 0-3, pg 977)
	HWREG(SSI0_BASE + SSI_O_IM) |= SSI_IM_EOTIM; 

	// Set NVIC enable
	HWREG(NVIC_EN0) |= BIT7HI;
	
	// Make sure interrupts are enables globally
	__enable_irq( );
	
	// Make sure that the SSI is enabled for operation
	HWREG(SSI0_BASE + SSI_O_CR1) |= SSI_CR1_SSE;
	
	// Print to console if successful initialization
	printf("DRS SPI Interface Initialized\n\r");
	
	// Check if we are running in debug mode and print to the console
	#ifdef TEST
		printf("   Test mode is enabled!\n\r");
	#endif /* If we are setting commands manually */
	
	
	/************* Initialize pins to read KART# switch here ***************/
	// For now, assume we are KART1
	MY_KART = 1;
	
	// Set the CurrentState to Ready
	CurrentState = DRS_Ready;
	
	// Run the DRS State Machine
	if(CurrentEvent.EventType == ES_ENTRY)
	{
		ES_Event NewEvent = { EV_DRSNewQuery, 0 };
		RunDRS( NewEvent );
	}
}

/****************************************************************************
 Function
	PostDRS 

 Parameters
    ES_Event ThisEvent ,the event to post to the queue

 Returns
    bool false if the Enqueue operation failed, true otherwise

 Description
    Posts an event to this state machine's queue
****************************************************************************/
bool PostDRS ( ES_Event ThisEvent )
{
	
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
	RunDRS 

 Parameters
	ES_Event : the event to process

 Returns
	ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
	
****************************************************************************/
ES_Event RunDRS ( ES_Event CurrentEvent )
{
	bool MakeTransition = false; // Assume no state transition
	DRSState_t NextState = CurrentState;
	ES_Event ReturnEvent = {ES_NO_EVENT};
	
	//printf("Event posted to SPI \r\n");
	
	// Change this to be implemented in the start function
	if (CurrentEvent.EventType == ES_INIT)
	{
		// Set the initial query event
		
	}
	
	switch ( CurrentState )
	{
		
		case DRS_Ready : 
			// Execute During function for Ready state
			CurrentEvent = DuringDRS_Ready( CurrentEvent );
			// Check if the event is active
			if ( CurrentEvent.EventType != ES_NO_EVENT )
			{
				switch ( CurrentEvent.EventType )
				{
					case EV_DRSNewQuery : 
						// Select the next query to send to the RS
						CurrentQuery = DRSQuerySelect();
						
						if( DRSSendQuery() )
						{
							// Write to SPI was successful, move to next state
							printf("SPI Write Successful\n\r");
							NextState = DRS_Transfer;
							MakeTransition = true;
						}
						else
						{
							// Write failed to SPI Data register, try again by
							// resetting the query to the last successful transfer
							// and running the state machine again
							printf("SPI Write Failed: Retry\n\r");
							NextState = DRS_Wait;
							MakeTransition = true;
							
							// Reset current query to last successful query
							CurrentQuery = LastQuery;
							
						}
						break;
				}
			}
			break;
			
		case DRS_Transfer : 
			// Execute During function for Ready state
			CurrentEvent = DuringDRS_Transfer( CurrentEvent );
			// Check if the event is active
			if ( CurrentEvent.EventType != ES_NO_EVENT )
			{
				switch ( CurrentEvent.EventType )
				{
					// NewRead is posted by the EOT interrupt after saving the information
					case EV_DRSNewRead : 
						if( DRSSaveData() )
						{
							// SaveData was successful, move to next state
							NextState = DRS_Wait;
							MakeTransition = true;
						}
						else
						{
							// No new data was read, repeat last query by 
							// setting current state to Wait and resend query after 3ms timeout
							printf("SPI Read Failed: Retry sending previous query\r\n");
							NextState = DRS_Wait;
							MakeTransition = true;
							
							// Reset current query to last successful query
							CurrentQuery = LastQuery;
						}
						break;
						
					case ES_TIMEOUT : 
						// No EOT interrupt received, if EOTResponseFlag is false repeat last 
						// query by setting current state to Wait and modifying CurentEvent to
						// resend the failed query
						if( EOTResponseFlag == false)
						{
							printf("SPI Timeout: Retry sending previous query\r\n");
							NextState = DRS_Wait;
							MakeTransition = true;
							
							// Reset current query to last successful query
							CurrentQuery = LastQuery;
						}
					
						break;
				}
			}
			break;
			
		case DRS_Wait : 
			// Execute During function for Ready state
			CurrentEvent = DuringDRS_Wait( CurrentEvent );
			// Check if the event is active
			if ( CurrentEvent.EventType != ES_NO_EVENT )
			{
				switch ( CurrentEvent.EventType )
				{
					case ES_TIMEOUT : 
							// Delay of 3ms successful, set to Ready
							NextState = DRS_Ready;
							MakeTransition = true;
						break;
				}
			}
			break;
	}
	
	// If we are making a state transition
	if ( MakeTransition == true )
	{
		// Execute exit function for the current state
		CurrentEvent.EventType = ES_EXIT;
		RunDRS(CurrentEvent);
		
		// Modify current state
		CurrentState = NextState;
		
		// Execute entry function for new state
		CurrentEvent.EventType = ES_ENTRY;
		RunDRS(CurrentEvent);
	}
  return ReturnEvent;
}


/****************************************************************************
 Function
   EOTResponse

 Parameters
   none

 Returns
   none

 Description
	End of Transfer interrupt response, checks command received from Reckoning 
	System and updates the game state or KART positions
****************************************************************************/
void EOTResponse( void ) 
{
	int Index;
	
	// Set the EOTResponseFlag to true
	EOTResponseFlag = true;
	
	#ifndef TEST
	// Make sure the SPI it not transmitting before reading receive register
	if( (HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_BSY) != SSI_SR_BSY )
	{
		// Read 8 bytes received from the RS into NewDRSRead
		for (Index = 0; Index<8; Index++)
		{
			NewDRSRead[Index] = HWREG(SSI0_BASE + SSI_O_DR);
		}	
	}
	#endif // Skips if we are setting commands manually
	
	// Post NewRead event to DRS
	ES_Event NewEvent = {EV_DRSNewRead};
	PostMaster(NewEvent);
}

/****************************************************************************
 Function
	QueryDRS

 Parameters
   none

 Returns
   RSState_t

 Description
   Returns the state of the Reckoning state machine which will be one of:
			RS_Ready
			RS_Transfer
			RS_Wait
****************************************************************************/
DRSState_t QueryDRS ( void )
{
	return CurrentState;
}

/****************************************************************************
 Function
	QueryMyKart

 Parameters
   none

 Returns
   KART_t

 Description
   Returns the following information for MyKart:
			uint16_t 		KartX;
			uint16_t 		KartY;
			uint16_t 		KartTheta;
			uint8_t			LapsRemaining;
			bool				ShotComplete;
			bool				ObsticaleeComplete;
			GameState_t	GameState;
****************************************************************************/
KART_t QueryMyKart ( void )
{
	if( MY_KART == 1 ) return Kart1;
	else if( MY_KART == 2 ) return Kart2;
	else return Kart3;
}

/***************************************************************************
 private functions
 ***************************************************************************/
/****************************************************************************
 Function
   DRSSendQuery

 Parameters
   none

 Returns
   none

 Description
   Query the Reckoning System (RS), set the current RS_CurrentQueryState
****************************************************************************/
static bool DRSSendQuery( void ) 
{
	// Initialize ReturnFlag as false (in case of unsuccessful transfer init)
	bool ReturnFlag = false;
	int i;
	
	// Check if the data output FIFO queue is empty
	if( (HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_TFE) == SSI_SR_TFE )
	{
		// Write query byte to the data output register
		HWREG(SSI0_BASE + SSI_O_DR) = CurrentQuery;
	
		// Write 0x00 to the data output register 7 times
		for (i = 1; i<8; i++)
		{
			HWREG(SSI0_BASE + SSI_O_DR) = 0x00;
		}	
	
		// Set the SSI transmit interrupt as unmasked to enable EOT interrupt (SSI_IM_TXIM)
		HWREG(SSI0_BASE + SSI_O_IM) |= BIT3HI;
	
		// Set EOTResponse flag false and start timer for transfer time-out
		EOTResponseFlag = false;
		
		// Set ReturnFlag to true to indicate a successful write
		ReturnFlag = true;
	}
		
	// Return if write was successful
	return ReturnFlag;
}

/****************************************************************************
 Function
   DRSQuerySelect

 Parameters
   none

 Returns
   none

 Description
   Query the Reckoning System (RS), set the current RS_CurrentQueryState
****************************************************************************/
static uint8_t DRSQuerySelect( void )
{
	uint8_t NextQuery;
	LastQuery = CurrentQuery;
	
	// Set the CurrentQueryState for the next transfer
	// Only switching between GameState and KART1 for now
	switch(LastQuery)
	{
		case QUERY_GAME_STATE : 
			NextQuery = QUERY_KART1;
			break;
		
		case QUERY_KART1 : 	
			NextQuery = QUERY_GAME_STATE;
			break;
		case QUERY_KART2 :				
			NextQuery = QUERY_KART2;

			break;
		case QUERY_KART3 : 	
			NextQuery = QUERY_KART3;
			break;
		default : NextQuery = QUERY_GAME_STATE; // If initializing
	}
	
	return NextQuery;
}

/****************************************************************************
 Function
   DRSSaveData

 Parameters
   none

 Returns
   bool true if successful

 Description
   Takes the 8 byte response from the RS and saves the information based
   on the current query
****************************************************************************/
static bool DRSSaveData ( void)
{
	bool ReturnVal = false;
	
	// Check that the NewDRSRead wasn't a response to an invalid command
	if( (NewDRSRead[1] & NewDRSRead[2] & NewDRSRead[3] & NewDRSRead[4] & NewDRSRead[5] & NewDRSRead[6] & NewDRSRead[7]) != INVALID_READ)
	{
		ReturnVal = true;
		
		// Save the data read from the RS to the appropriate variables
		switch(CurrentQuery)
		{
			case QUERY_GAME_STATE  :
			{
				// Save the CurrentGameState for each of the KARTS
				uint8_t Kart1State = NewDRSRead[3];
				//uint8_t Kart2State = NewDRSRead[4];
				//uint8_t Kart3State = NewDRSRead[5];
		
				// Cycle through GameState and pull GameState information for each KART
				// Assume we are KART1 for now (will update for KART variability later)
				// Start by setting GameState
				uint8_t Kart1GameState = ( (Kart1State & GAME_STATE_MASK)>>3);
				switch( Kart1GameState )
				{
					case WAIT_FOR_START : Kart1.GameState = DRS_WaitingForStart; break;
					case FLAG_DROPPED : 	Kart1.GameState = DRS_FlagDropped; break;
					case CAUTION_FLAG : 	Kart1.GameState = DRS_CautionFlag; break;
					case RACE_OVER : 			Kart1.GameState = DRS_RaceOver; break;
				}
			
				// Then set number of laps remaining (is this zero based or unit based???)
				// e.g. LastLap = 0 or LastLap = 1??
				Kart1.LapsRemaining = (Kart1State & LAPS_REMAINING_MASK);
						
				// Then check if our robot has sucessfully made a shot into the bucket
				if( (Kart1State & SHOT_COMPLETE) == SHOT_COMPLETE ) Kart1.ShotComplete = true;
				else Kart1.ShotComplete = false;
				
				// Finally check if sea-saw has been succesfully navigated
				if( (Kart1State & OBSTICAL_COMPLETE) == OBSTICAL_COMPLETE ) Kart1.ObstacleComplete = true;
				else Kart1.ObstacleComplete = false;
				
				// No more data to save, print the new GameState for KART1
				// since we are assuming we are KART1 for now
				printf("GameState received for KART1:\r\n");
				printf("   GameState:     %d\r\n", Kart1.GameState);
				printf("   LapsRemaining: %d\r\n", Kart1.LapsRemaining);
				printf("   ShotMade:      %d\r\n", Kart1.ShotComplete);
				printf("   Obsticale:      %d\r\n", Kart1.ObstacleComplete);
				break;
			}
			
			case QUERY_KART1 : 
			{
				// Save the position and orientation of KART1
				// Cycle through NewDRSRead and pull KART1 information
				Kart1.KartX = ((NewDRSRead[2]<<8) + NewDRSRead[3]);
				Kart1.KartY = ((NewDRSRead[4]<<8) + NewDRSRead[5]);
				Kart1.KartTheta = ((NewDRSRead[6]<<8) + NewDRSRead[7]);
				
				// No more data to save, print the new KART information
				printf("KART1 position/orientation received:\r\n");
				printf("   KARTX:     %d\r\n", Kart1.KartX);
				printf("   KARTY:     %d\r\n", Kart1.KartY);
				printf("   KARTTheta: %d\r\n", Kart1.KartTheta);
				break;
			}
			
			case QUERY_KART2 :
			{
				// Save the position and orientation of KART1
				// Cycle through NewDRSRead and pull KART1 information
				Kart2.KartX = ((NewDRSRead[2]<<8) + NewDRSRead[3]);
				Kart2.KartY = ((NewDRSRead[4]<<8) + NewDRSRead[5]);
				Kart2.KartTheta = ((NewDRSRead[6]<<8) + NewDRSRead[7]);
				
				// No more data to save, print the new KART information
				printf("KART2 position/orientation received:\r\n");
				printf("   KARTX:     %d\r\n", Kart2.KartX);
				printf("   KARTY:     %d\r\n", Kart2.KartY);
				printf("   KARTTheta: %d\r\n", Kart2.KartTheta);
				break;
			}
			
			case QUERY_KART3 :
			{
				// Save the position and orientation of KART1
				// Cycle through NewDRSRead and pull KART1 information
				Kart3.KartX = ((NewDRSRead[2]<<8) + NewDRSRead[3]);
				Kart3.KartY = ((NewDRSRead[4]<<8) + NewDRSRead[5]);
				Kart3.KartTheta = ((NewDRSRead[6]<<8) + NewDRSRead[7]);
				
				// No more data to save, print the new KART information
				printf("KART3 position/orientation received:\r\n");
				printf("   KARTX:     %d\r\n", Kart3.KartX);
				printf("   KARTY:     %d\r\n", Kart3.KartY);
				printf("   KARTTheta: %d\r\n", Kart3.KartTheta);
				break;
			}
		}
	}
	return ReturnVal;
}
/****************************************************************************
 Function
   DuringDRS_Ready

 Parameters
   ES_Event

 Returns
   ES_Event

 Description
   do nothing
****************************************************************************/
static ES_Event DuringDRS_Ready( ES_Event Event)
{
	// Process EV_ENTRY and EV_EXIT events
	if ( Event.EventType == ES_ENTRY )
	{
		//printf("DuringReady: Entry\n\r");
	}
	else if ( Event.EventType == ES_EXIT )
	{
		//printf("DuringReady: Exit\n\r");
	}
	else
	{
		// Do the 'during' function for this state
	}
	return Event;
}

/****************************************************************************
 Function
   DuringDRS_Transfer

 Parameters
   ES_Event

 Returns
   ES_Event

 Description
   On entry, start transfer timeout timer. On exit, stop the timer.
****************************************************************************/
static ES_Event DuringDRS_Transfer( ES_Event Event)
{
	// Process EV_ENTRY and EV_EXIT events
	if ( Event.EventType == ES_ENTRY ) 
	{
		//printf("DuringTransfer: Entry\n\r");
		ES_Timer_InitTimer(DRS_TIMER, DRS_COMMAND_TIMEOUT);
	}
	else if ( Event.EventType == ES_EXIT ) 
	{
		//printf("DuringTransfer: Exit\n\r");
		ES_Timer_StopTimer(DRS_TIMER);
	}
	else
	{
		// Do the 'during' function for this state
	}
	return Event;
}

/****************************************************************************
 Function
   DuringDRS_Wait

 Parameters
   ES_Event

 Returns
   ES_Event

 Description
   On entry, start wait timer. On exit, do nothing.
****************************************************************************/
static ES_Event DuringDRS_Wait( ES_Event Event)
{
	
	// Process EV_ENTRY and EV_EXIT events
	if ( Event.EventType == ES_ENTRY ) 
	{
		// Start the timer for delay between RS reads
		//printf("DuringWait: Entry\n\r");
		ES_Timer_InitTimer(DRS_TIMER, DRS_COMMAND_DELAY);
	}
	else if ( Event.EventType == ES_EXIT ) 
	{
		// Post RSNewQuery upon leaving Wait state
		//printf("DuringWait: Exit\n\r");
		ES_Event NewEvent = { EV_DRSNewQuery, 0 };
		PostMaster(NewEvent);
	}
	else
	{
		// Do the 'during' function for this state
	}
	return Event;
}
/*------------------------------- Footnotes -------------------------------*/

/*------------------------------ End of file ------------------------------*/

