#ifndef DriveAlgorithim_H
#define DriveAlgorithim_H

#include <math.h>
#include <stdint.h>
#include <stdbool.h>

void addAngleEntry(uint16_t thetaNew);
int getDesiredTheta(void);
void clearThetas(void);
int QueryTheta( void );

static bool AtJunction(void);


#endif 


