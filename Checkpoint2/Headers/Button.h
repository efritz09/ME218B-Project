/****************************************************************************
 
  Header file for Button

 ****************************************************************************/

#ifndef Button_H
#define Button_H

#include "ES_Types.h"

// Define States
typedef enum {DEBOUNCING, READY2SAMPLE} ButtonState;

// Public Function Prototypes

bool InitButton(uint8_t Priority);
bool PostButton(ES_Event ThisEvent);
ES_Event RunButton(ES_Event ThisEvent);
bool CheckButtonEvents( void );

void setLastButtonState(uint8_t last);
uint8_t getLastButtonState(void);


#endif /* AD_H */
