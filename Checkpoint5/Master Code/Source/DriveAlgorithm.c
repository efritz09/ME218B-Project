#include "Headers.h"

#define maxSize 5

static uint16_t size = 0;
static int thetas[maxSize]; 

/*public functions*/

/*takes in a value and adds it into an array to keep track of DRS data*/
void addAngleEntry(uint16_t thetaNew){
	if(size<maxSize)/*if you have not reached the max number of data points */
		{
		// add your data point into the array
		thetas[size] = (int)thetaNew;
		// increase the number of data points aquired
		size++;
	}
	else/*if you have reached the maximum*/{
		// shift all the data points up 
		for(int i =maxSize-1; i>0; i--){
			thetas[i] = thetas[i-1];
		}	
		// add the new data point
		thetas[0] = (int)thetaNew;
	}
	
	//printf("DRS value:  %d, Moving AVG:  %d\n\r", thetaNew, getDesiredTheta());
}


int QueryTheta( void )
{
	return getDesiredTheta();
}

/*reurns a "smoothed" theta value*/
int getDesiredTheta(void){
	// if no data has been captured return -1 to indicate such
	if (size<1) return -1;
	
	//initialize a total value 
	int total = 0;
	
	if(AtJunction()){
		for(int i = 0;i<size; i++){
			if(thetas[i]<180){
				thetas[i]+=360;
			}
		}
	}
	//add all the relavent values in theta together
	for(int i = 0;i<size; i++){
		total+= thetas[i];
	}
	
	// divide by the number of theta entries for an average
	int avg = (total/size);
	if(avg>=360){
		return (avg-360);
	}
	return avg;
}


/*resets the number of data points to zero*/
void clearThetas(void){
	size = 0;
}






/*private functions*/

/*returns true if we get values on both sizes of 0 ie. 5 deg and 355 deg*/
static bool AtJunction(void){
	bool one = false;
	bool two = false;
	
	//printf("test at junction\n\r");	
	for(int i = 0;i<size-1; i++){
		if(thetas[i]<90){
			one =  true;
		}	else if(thetas[i]>270){
			two =  true;
		}
	}

	if(one && two) { //printf("At junction\r\n");
		return (one && two);
	}else {
		return false;
	}
}





