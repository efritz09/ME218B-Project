/****************************************************************************
 Module
	DriveAlgorithm.c

 Revision			Revised by: 
	0.1.1				Lizzie
	0.2.1				Denny

 Description
	Moving average with modulus of 360 to smooth theta from the DRS

 Edits:
	0.1.1 - Set up smoothing algorithm
	0.2.1 - Final code for submission
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "Headers.h"

/*----------------------------- Module Defines ----------------------------*/
#define maxSize 5

/*---------------------------- Module Functions ---------------------------*/
static bool AtJunction(void);

/*---------------------------- Module Variables ---------------------------*/
static uint16_t size = 0;
static int thetas[maxSize]; 

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
	addAngleEntry 

 Parameters
	uint16_t New theta value

 Returns
	none

 Description
	Takes in a value and adds it into an array to keep track of DRS data
****************************************************************************/
void addAngleEntry(uint16_t thetaNew){
	// If max number of data points has not been reached
	if(size<maxSize)
	{
		// add data point to the array and increase the size
		thetas[size] = (int)thetaNew;
		size++;
	}
	// If max number has been reached
	else
	{
		// Shift all the data points up and insert new entry
		for(int i =maxSize-1; i>0; i--)
		{
			thetas[i] = thetas[i-1];
		}	
		thetas[0] = (int)thetaNew;
	}
}

/****************************************************************************
 Function
	QueryTheta 

 Parameters
	none

 Returns
	int

 Description
	Returns the average of the theta array with a modulus of 360
****************************************************************************/
int QueryTheta( void )
{
	return getDesiredTheta();
}


/****************************************************************************
 Function
	getDesiredTheta 

 Parameters
	none

 Returns
	int

 Description
	Returns the average of the theta array with a modulus of 360
****************************************************************************/
int getDesiredTheta(void)
{
	// Initialize our total sum 
	int total = 0;
	
	// Check if any points in the array are both above/below the modulus
	if(AtJunction())
	{
		for(int i = 0;i<size; i++){
			if(thetas[i]<180){
				thetas[i]+=360;
			}
		}
	}
	
	//add all the relavent values in theta together
	for(int i = 0;i<size; i++)
	{
		total+= thetas[i];
	}
	
	// divide by the number of theta entries for an average
	int avg = (total/size);
	if(avg>=360){
		return (avg-360);
	}
	return avg;
}

/****************************************************************************
 Function
	clearThetas 

 Parameters
	none

 Returns
	none

 Description
	Resets the number of data points to zero
****************************************************************************/
void clearThetas(void){
	size = 0;
}

/***************************************************************************
 private functions
 ***************************************************************************/
/****************************************************************************
 Function
	AtJunction 

 Parameters
	none

 Returns
	int

 Description
	Check if any points in the array are both above/below the modulus
****************************************************************************/
static bool AtJunction(void)
{
	bool Upper = false;
	bool Lower = false;
	
	for(int i = 0;i<size-1; i++)
	{
		if(thetas[i]<90){
			Lower =  true;
		}	else if(thetas[i]>270){
			Upper =  true;
		}
	}

	if(Upper && Lower) return true;
	else return false;
}
