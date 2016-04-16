/****************************************************************************
 Module
   PWM.c
****************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_pwm.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "bitdefs.h"

#include "PWM.h"

// 40,000 ticks per mS assumes a 40Mhz clock, we will use SysClk/32 for PWM
//set /32 for up to 1000Hz
//set /32000 for microseconds
#define PWMTicksPerMS 40000/32000
// set 200 Hz frequency so 5mS period
// set 250 Hz frequency so 4mS period
// set 500 Hz frequency so 2mS period
// set 1000 Hz frequency so 1mS period
// set 2000 Hz frequency so 500us
// set 10000 Hz frequency so 100us
#define PeriodInMS 315 //Motor time constant is 315us
#define ALL_BITS (0xff<<2)
#define BitsPerNibble 4

//Init PWM pins 6 & 7. 6 will be right, 7 will be left
void InitPWM( void ){
 volatile uint32_t Dummy; // use volatile to avoid over-optimization
// start by enabling the clock to the PWM Module (PWM0)
  HWREG(SYSCTL_RCGCPWM) |= SYSCTL_RCGCPWM_R0;

// enable the clock to Port B  
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;

// Select the PWM clock as System Clock/32
  HWREG(SYSCTL_RCC) = (HWREG(SYSCTL_RCC) & ~SYSCTL_RCC_PWMDIV_M) |
    (SYSCTL_RCC_USEPWMDIV | SYSCTL_RCC_PWMDIV_32);
  
// make sure that the PWM module clock has gotten going
	while ((HWREG(SYSCTL_PRPWM) & SYSCTL_PRPWM_R0) != SYSCTL_PRPWM_R0)
    ;

// disable the PWM while initializing
  HWREG( PWM0_BASE+PWM_O_0_CTL ) = 0;

// program generator A to go to 0 at rising comare A, 1 on falling compare A  
  HWREG( PWM0_BASE+PWM_O_0_GENA) = 
    (PWM_0_GENA_ACTCMPAU_ZERO | PWM_0_GENA_ACTCMPAD_ONE );

// program generator B to go to 0 at rising comare B, 1 on falling compare B  
  HWREG( PWM0_BASE+PWM_O_0_GENB) = 
    (PWM_0_GENA_ACTCMPBU_ZERO | PWM_0_GENA_ACTCMPBD_ONE );

// Set the PWM period. Since we are counting both up & down, we initialize
// the load register to 1/2 the desired total period
  HWREG( PWM0_BASE+PWM_O_0_LOAD) = ((PeriodInMS * PWMTicksPerMS)-1)>>1;
  
// Set the initial Duty cycle on A to 50% by programming the compare value
// to 1/2 the period to count up (or down) 
  HWREG( PWM0_BASE+PWM_O_0_CMPA) = ((PeriodInMS * PWMTicksPerMS)-1)>>2;

// Set the initial Duty cycle on B to 25% by programming the compare value
// to 1/4 the period  
  HWREG( PWM0_BASE+PWM_O_0_CMPB) = ((PeriodInMS * PWMTicksPerMS)-1)>>3;

// set changes to the PWM output Enables to be locally syncronized to a 
// zero count
  HWREG(PWM0_BASE+PWM_O_ENUPD) =  (HWREG(PWM0_BASE+PWM_O_ENUPD) & 
      ~(PWM_ENUPD_ENUPD0_M | PWM_ENUPD_ENUPD1_M)) |
      (PWM_ENUPD_ENUPD0_LSYNC | PWM_ENUPD_ENUPD1_LSYNC);

// enable the PWM outputs
  HWREG( PWM0_BASE+PWM_O_ENABLE) |= (PWM_ENABLE_PWM1EN | PWM_ENABLE_PWM0EN);

// now configure the Port B pins to be PWM outputs
// start by selecting the alternate function for PB6 & 7
  HWREG(GPIO_PORTB_BASE+GPIO_O_AFSEL) |= (BIT7HI | BIT6HI);

// now choose to map PWM to those pins, this is a mux value of 4 that we
// want to use for specifying the function on bits 6 & 7
  HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) = 
    (HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) & 0x00fffff) + (4<<(7*BitsPerNibble)) +
      (4<<(6*BitsPerNibble));

// Enable pins 4, 5, 6, & 7 on Port B for digital I/O
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (BIT7HI | BIT6HI | BIT5HI | BIT4HI | BIT3HI | BIT2HI);
	
// make pins 4, 5, 6, & 7 on Port B into outputs
	HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= (BIT7HI | BIT6HI | BIT5HI | BIT4HI | BIT3HI | BIT2HI);
  SetPWMDuty(0,0);
	SetPWMDuty(0,1);
	HWREG(GPIO_PORTB_BASE + ALL_BITS) &= ~(BIT4HI | BIT5HI);
	
// set the up/down count mode and enable the PWM generator
  HWREG(PWM0_BASE+ PWM_O_0_CTL) |= (PWM_0_CTL_MODE | PWM_0_CTL_ENABLE);
	puts("PWM Initialized\n\r");
}


void SetPWMDuty(uint8_t duty, int channel) {
	//printf("duty cycle = %d\r\n", duty);
	int newDuty;
	static bool zeroStatus = false;
	//check for 100 or 0 duty
	if(duty == 0 || duty > 100) {
		newDuty = 0;
		zeroStatus = true;
	}
	else {
		newDuty = ((PeriodInMS * PWMTicksPerMS) - 1)*duty/100;
		zeroStatus = false; 
	}

	switch (channel) {
		case 0 : //PB6
			if(zeroStatus){ //disable the output
				HWREG( PWM0_BASE+PWM_O_ENABLE) &= ~PWM_ENABLE_PWM0EN;
			}else { //ensure output is enabled
				HWREG( PWM0_BASE+PWM_O_ENABLE) |= PWM_ENABLE_PWM0EN;
			}
			HWREG( PWM0_BASE+PWM_O_0_CMPA) = (newDuty)>>1; //write value
			break;
		case 1 : //PB7
			if(zeroStatus){ //same shit as above, different pwm channel
				HWREG( PWM0_BASE+PWM_O_ENABLE) &= ~PWM_ENABLE_PWM1EN;
			}else {
				HWREG( PWM0_BASE+PWM_O_ENABLE) |= PWM_ENABLE_PWM1EN;
			}
			HWREG( PWM0_BASE+PWM_O_0_CMPB) = (newDuty)>>1;
			break;
	}
}
