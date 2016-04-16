/****************************************************************************
 
  Header file for the drive system

 ****************************************************************************/

#ifndef Drive_H
#define Drive_H

#include <stdint.h>
#include "Driving.h"

/*----------------------- Public Function Prototypes ----------------------*/
void DriveTo( uint16_t X, uint16_t Y );
void TurnTheta( float theta );
bool CheckVal ( uint16_t val, int select);
MapSection FindNextSection( POINT_t current );

ES_Event RunDrive ( ES_Event CurrentEvent );
void StartDrive (ES_Event CurrentEvent );
#endif /* Drive_H */
