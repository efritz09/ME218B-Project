/****************************************************************************
 
  Header file the DrEd Reckoning System SPI State Machine (RS)

 ****************************************************************************/
#ifndef DriveAlgorithim_H
#define DriveAlgorithim_H

/*----------------------- Public Function Prototypes ----------------------*/
void addAngleEntry(uint16_t thetaNew);
int getDesiredTheta(void);
void clearThetas(void);
int QueryTheta( void );

#endif /* DriveAlgorithim_H */
