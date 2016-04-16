/****************************************************************************
Header file for Hierarchical Sate Machines AKA StateCharts

 ****************************************************************************/

#ifndef MasterMachine_H
#define MasterMachine_H

// Public Function Prototypes

ES_Event RunMaster( ES_Event CurrentEvent );
void StartMaster ( ES_Event CurrentEvent );
bool PostMaster( ES_Event ThisEvent );
bool InitMaster ( uint8_t Priority );

#endif /*MasterMachine_H */

