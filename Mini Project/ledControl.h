#ifndef LEDCONTROL_H
#include "MKL25Z4.h"

#define LEDCONTROL_H
#define MASK(x)         (1 << (x)) 

//Define pin numbers
#define RED_LED_1  				29 // PortE Pin 29 
#define GREEN_LED_1       7  // PortC Pin 7 
#define GREEN_LED_2       0  // PortC Pin 0 
#define GREEN_LED_3       3  // PortC Pin 3 
#define GREEN_LED_4       4  // PortC Pin 4 
#define GREEN_LED_5       5  // PortC Pin 5 
#define GREEN_LED_6       6  // PortC Pin 6 
#define GREEN_LED_7       10 // PortC Pin 10 
#define GREEN_LED_8      	11 // PortC Pin 11
#define MASK_GREEN_LED_ALL 0x00000CF9

// Delay for different modes
#define MOVING_DELAY				500		// 500 ms
#define STATIONARY_DELAY 		250		// 250 ms


int greenLEDStrip[8] = {GREEN_LED_1, GREEN_LED_2, GREEN_LED_3, GREEN_LED_4, 
												GREEN_LED_5, GREEN_LED_6, GREEN_LED_7, GREEN_LED_8};

void initLED(void);

void ledTest(void) {
	PTC->PDOR |= MASK(GREEN_LED_1);
	PTE->PDOR |= MASK(RED_LED_1);
}

void initLED(void) { 
	// Enable Clock to PORTB and PORTD 
	SIM->SCGC5 |= ((SIM_SCGC5_PORTC_MASK) | (SIM_SCGC5_PORTE_MASK)); 

	// Configure MUX settings to make all 3 pins GPIO 
	PORTC->PCR[GREEN_LED_1] &= ~PORT_PCR_MUX_MASK; 
	PORTC->PCR[GREEN_LED_1] |= PORT_PCR_MUX(1); 
	
	PORTC->PCR[GREEN_LED_2] &= ~PORT_PCR_MUX_MASK; 
	PORTC->PCR[GREEN_LED_2] |= PORT_PCR_MUX(1); 
	
	PORTC->PCR[GREEN_LED_3] &= ~PORT_PCR_MUX_MASK; 
	PORTC->PCR[GREEN_LED_3] |= PORT_PCR_MUX(1); 
	
	PORTC->PCR[GREEN_LED_4] &= ~PORT_PCR_MUX_MASK; 
	PORTC->PCR[GREEN_LED_4] |= PORT_PCR_MUX(1); 
	
	PORTC->PCR[GREEN_LED_5] &= ~PORT_PCR_MUX_MASK; 
	PORTC->PCR[GREEN_LED_5] |= PORT_PCR_MUX(1); 
	
	PORTC->PCR[GREEN_LED_6] &= ~PORT_PCR_MUX_MASK; 
	PORTC->PCR[GREEN_LED_6] |= PORT_PCR_MUX(1); 
	
	PORTC->PCR[GREEN_LED_7] &= ~PORT_PCR_MUX_MASK; 
	PORTC->PCR[GREEN_LED_7] |= PORT_PCR_MUX(1); 
	
	PORTC->PCR[GREEN_LED_8] &= ~PORT_PCR_MUX_MASK; 
	PORTC->PCR[GREEN_LED_8] |= PORT_PCR_MUX(1); 

	PORTE->PCR[RED_LED_1] &= ~PORT_PCR_MUX_MASK; 
	PORTE->PCR[RED_LED_1] |= PORT_PCR_MUX(1); 

	// Set Data Direction Registers for PortC and PortE Pins to Output
	PTC->PDDR |= MASK(GREEN_LED_1);
	PTC->PDDR |= MASK(GREEN_LED_2);
	PTC->PDDR |= MASK(GREEN_LED_3);
	PTC->PDDR |= MASK(GREEN_LED_4);
	PTC->PDDR |= MASK(GREEN_LED_5);
	PTC->PDDR |= MASK(GREEN_LED_6);
	PTC->PDDR |= MASK(GREEN_LED_7);
	PTC->PDDR |= MASK(GREEN_LED_8);

	PTE->PDDR |= MASK(RED_LED_1); 
}
												
#endif
