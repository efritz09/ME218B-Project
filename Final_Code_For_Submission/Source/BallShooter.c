/****************************************************************************
 Module
   BallShooter.c

 Revision			Revised by: 
	0.1.1					Eric			

 Description
  State Machine for launching and reloading the balls
	 
 Edits:

 Notes:
	Ball Servo Limits: Width = 500-2500 (moves ~ 180 degrees)
	Hopper Servo Limits: Width = 500-2000 (moves ~ 180 degrees)
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
#include "Headers.h"

/*----------------------------- Module Defines ----------------------------*/
// Symbolic defines for time
#define BALL_TIME ONE_SEC

// Symbolic defines for servo positions/PWM channel
#define SHOOTER_SET						1400
#define SHOOTER_FLICK					850	
#define SHOOTER_RESET					1900	
#define SHOOTER_CHANNEL				2
#define HOPPER_SET						1000
#define HOPPER_RELEASE				1500
#define HOPPER_CHANNEL				3

/*---------------------------- Module Functions ---------------------------*/
static void FlickerSetStartPosition( void );
static void FlickerFlickBall( void );
static void FlickerReset( void );
static void HopperSet( void );
static void HopperRelease( void );

/*---------------------------- Module Variables ---------------------------*/
static ShooterState BallShootState;

/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
	InitBallShooter

 Parameters
	none

 Returns
	Bool indicating success

 Description
	Initializes everything for the shooter
****************************************************************************/
bool InitBallShooter(void)
{
	// Put both servos into their default positions
	FlickerReset(); 
	HopperSet();
	
	// Set current state to ShooterResetting
	BallShootState = ShooterResetting;
	ES_Timer_InitTimer(BALL_SHOOTER_TIMER,BALL_TIME);
	printf("Ball Shooter Initialized\r\n");
	return true;
}

/****************************************************************************
 Function
	RunBallShooter

 Description
  Runs a pseudo-state machine to test our servo motors
****************************************************************************/
ES_Event RunBallShooter ( ES_Event ThisEvent )
{
	ES_Event ReturnEvent = {ES_NO_EVENT};
	
	if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == BALL_SHOOTER_TIMER) {
		switch (BallShootState)
		{
			case ShooterReady:
				// Reset after shot
				FlickerReset();
				BallShootState = ShooterResetting;
				ES_Timer_InitTimer(BALL_SHOOTER_TIMER,BALL_TIME);
				break;
			case ShooterResetting:
				// Set the start position and reload the ball
				FlickerSetStartPosition();
				BallShootState = ShooterReady;
				break;
		}
	}
	return ReturnEvent;
}

/****************************************************************************
 Function
	FireBallShooter
 Description
	Moves ball shooting servo to shooting position to flick ruler and shoot ball
****************************************************************************/
void FireBallShooter(void)
{
	FlickerFlickBall();
	ES_Timer_InitTimer(BALL_SHOOTER_TIMER,BALL_TIME);
}

/****************************************************************************
 Function
	SetStartPosition

 Description
    
****************************************************************************/
static void FlickerSetStartPosition( void )
{
	SetPWMWidth( SHOOTER_SET, SHOOTER_CHANNEL);
	printf("Flicker is set to Start position\r\n");
	HopperRelease();
}

/****************************************************************************
 Function
	FlickBall

 Description
   
****************************************************************************/
static void FlickerFlickBall( void )
{
	SetPWMWidth( SHOOTER_FLICK, SHOOTER_CHANNEL);
	printf("Flicker has fired the ball\r\n");
}

/****************************************************************************
 Function
	Reset

 Description
    
****************************************************************************/
static void FlickerReset( void )
{
	SetPWMWidth( SHOOTER_RESET, SHOOTER_CHANNEL);
	printf("Flicker has been reset\r\n");
	HopperSet();
}

/****************************************************************************
 Function
	HopperSet

 Description
   
****************************************************************************/
static void HopperSet( void )
{
	SetPWMWidth( HOPPER_SET, HOPPER_CHANNEL);
	printf("hopper is set\r\n");
}

/****************************************************************************
 Function
	HopperRelease

 Description
    
****************************************************************************/
static void HopperRelease( void )
{
	SetPWMWidth( HOPPER_RELEASE, HOPPER_CHANNEL);
	printf("hopper is released\r\n");
}

/*
                 ."-,.__
                 `.     `.  ,
              .--'  .._,'"-' `.
             .    .'         `'
             `.   /          ,'
               `  '--.   ,-"'
                `"`   |  \
                   -. \, |
                    `--Y.'      ___.
                         \     L._, \
               _.,        `.   <  <\                _
             ,' '           `, `.   | \            ( `
          ../, `.            `  |    .\`.           \ \_
         ,' ,..  .           _.,'    ||\l            )  '".
        , ,'   \           ,'.-.`-._,'  |           .  _._`.
      ,' /      \ \        `' ' `--/   | \          / /   ..\
    .'  /        \ .         |\__ - _ ,'` `        / /     `.`.
    |  '          ..         `-...-"  |  `-'      / /        . `.
    | /           |L__           |    |          / /          `. `.
   , /            .   .          |    |         / /             ` `
  / /          ,. ,`._ `-_       |    |  _   ,-' /               ` \
 / .           \"`_/. `-_ \_,.  ,'    +-' `-'  _,        ..,-.    \`.
.  '         .-f    ,'   `    '.       \__.---'     _   .'   '     \ \
' /          `.'    l     .' /          \..      ,_|/   `.  ,'`     L`
|'      _.-""` `.    \ _,'  `            \ `.___`.'"`-.  , |   |    | \
||    ,'      `. `.   '       _,...._        `  |    `/ '  |   '     .|
||  ,'          `. ;.,.---' ,'       `.   `.. `-'  .-' /_ .'    ;_   ||
|| '              V      / /           `   | `   ,'   ,' '.    !  `. ||
||/            _,-------7 '              . |  `-'    l         /    `||
. |          ,' .-   ,' ||               | .-.        `.      .'     ||
 `'        ,'    `".'    |               |    `.        '. -.'       `'
          /      ,'      |               |,'    \-.._,.'/'
          .     /        .               .       \    .''
        .`.    |         `.             /         :_,'.'
          \ `...\   _     ,'-.        .'         /_.-'
           `-.__ `,  `'   .  _.>----''.  _  __  /
                .'        /"'          |  "'   '_
               /_|.-'\ ,".             '.'`__'-( \
                 / ,"'"\,'               `/  `-.|" 
*/
