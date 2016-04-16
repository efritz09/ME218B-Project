/****************************************************************************
 
  Header file for EncoderService

 ****************************************************************************/

#ifndef Enc_H
#define Enc_H

#include "ES_Types.h"

// Public Function Prototypes

bool InitEncoderService ( uint8_t Priority );
bool PostEncoderService( ES_Event ThisEvent );
ES_Event RunEncoderService( ES_Event ThisEvent );


#endif /* Enc_H */
