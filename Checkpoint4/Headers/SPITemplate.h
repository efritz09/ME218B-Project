/****************************************************************************
 
  Header file the DrEd Reckoning System SPI State Machine (RS)

 ****************************************************************************/

#ifndef DRS_H
#define DRS_H

/*----------------------------- Include Files -----------------------------*/
// set up a global header for the project
// will need: #include "SMEvents.h"

/*----------------------------- Module Defines ----------------------------*/
typedef enum {
			DRS_Ready,
			DRS_Transfer,
			DRS_Wait
} DRSState_t;

typedef enum {
			DRS_WaitingForStart,
			DRS_FlagDropped,
			DRS_CautionFlag,
			DRS_RaceOver
} GameState_t;

typedef struct {
			uint16_t 		KartX;
			uint16_t 		KartY;
			uint16_t 		KartTheta;
			uint8_t			LapsRemaining;
			bool				ShotComplete;
			bool				ObstacleComplete;
			GameState_t	GameState;
} KART_t;



/*----------------------- Public Function Prototypes ----------------------*/

bool InitDRS ( uint8_t Priority );
void StartDRS( ES_Event CurrentEvent );
bool PostDRS ( ES_Event ThisEvent );				// need to remove this for HSM
ES_Event RunDRS ( ES_Event CurrentEvent );
void EOTResponse( void );
DRSState_t QueryDRS ( void );
KART_t QueryMyKart ( void );

#endif /* DRS_H */
