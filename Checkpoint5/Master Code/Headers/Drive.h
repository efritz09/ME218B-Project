/****************************************************************************
 
  Header file for the drive system

 ****************************************************************************/

#ifndef Drive_H
#define Drive_H

#include "Headers.h"

/*----------------------- Public Function Prototypes ----------------------*/
void Calculate( uint16_t X, uint16_t Y );
void ExitZoneCalc ( int zoneNumber );
void DriveForward( void );
void TurnTheta( void );
bool CheckVal ( uint16_t val, int select);

ES_Event RunDrive ( ES_Event CurrentEvent );
void StartDrive (ES_Event CurrentEvent );
#endif /* Drive_H */
