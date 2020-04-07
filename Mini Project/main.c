/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/
 
#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
#include "motors.h"
#include "sound.h"
#include "ledControl.h"
#include "uart.h"

unsigned char rxData = 0x0;
volatile uint8_t dir = 0;
volatile uint8_t led_count = 0;

//  Prioritity of tBrain and tMotor
const osThreadAttr_t thread_attr = {
	.priority = osPriorityAboveNormal
};

osEventFlagsId_t data_flag;						// To Run tBrain when new data is received
osEventFlagsId_t dir_flag;						// To Run tControl after decoding of data

int songz[47] = {E4, E4, F4, G4, G4, F4, E4, D4, Z4, Z4, D4, E4, E4, D4, D4, E4, E4, F4, G4, G4, F4, E4, D4, Z4, Z4, D4, E4, D4, Z4, Z4, 
	D4, D4, E4, Z4, D4, E4, F4, E4, Z4, D4, E4, F4, E4, D4, Z4, D4, G3};

void UART2_IRQHandler(void) {
	NVIC_ClearPendingIRQ(UART2_IRQn);
	
	if(UART2->S1 & UART_S1_TDRE_MASK) {
	}	
	if(UART2->S1 & UART_S1_RDRF_MASK) {
		rxData = UART2->D;
		osEventFlagsSet(data_flag, 0x0002);
	}
}

/*----------------------------------------------------------------------------
 * 4 Application Threads
 *---------------------------------------------------------------------------*/
void tLED (void *argument) {
	//if flag set for running: led run function
	//if flag set for stationary: led stop function

	for(;;) {
		if (dir == 0x01 || dir == 0x00) { // stationary mode
			// 250ms on, off (red) & on continuously (green)
			PTE->PDOR = ~(PTE->PDOR);
			PTC->PDOR |= MASK_GREEN_LED_ALL;
			osDelay(250);
			
		} else { 													// running mode
			// 500ms on, off (red) & 1 at a time (green)
			PTC->PDOR &= ~MASK_GREEN_LED_ALL;
			PTE->PDOR = ~(PTE->PDOR);
			PTC->PDOR |= MASK(greenLEDStrip[led_count++]);
			osDelay(500);
			if (led_count >= 7) {
				led_count = 0;
			}
		}
	}
}

void tmusicControl(void *argument){
	for (;;){
		for(int i=0; i<SONG_LENGTH; i+=1){
			playSound(songz[i]);
			osDelay(NOTE_LENGTH);
			playSound(0);
			osDelay(BEAT);
		}
	}
}

void tBrain (void *argument) {
	// Decodes messages received from Serial Communication
	for(;;) {
		osEventFlagsWait(data_flag, 0x0002, osFlagsWaitAny, osWaitForever);
		osEventFlagsClear(data_flag, 0x0002);
		if (MASK_ON(rxData)) {
			dir |= rxData;
		} else {
			dir &= ~rxData;
		}
		
		// can add more for like buzzers
		osEventFlagsSet(dir_flag, 0x0001);
	}
}

void tMotorControl (void *argument) {
  // Controls direction of the Motors
  for (;;) {
		osEventFlagsWait(dir_flag, 0x0001, osFlagsWaitAny, osWaitForever); 		// Waits for dir to be updated by tBrain before it goes into READY state
		osEventFlagsClear(dir_flag, 0x0001);
		move(dir);
	}
}
 
int main (void) {
 
  // System Initialization
  SystemCoreClockUpdate();
	initMotors();
	initUART2();
	initLED();
	initPWM();
	
  // Initialize Kernel
  osKernelInitialize();                 						// Initialize CMSIS-RTOS
	dir_flag = osEventFlagsNew(NULL);
	data_flag = osEventFlagsNew(NULL);
  osThreadNew(tBrain, NULL, &thread_attr);    			// Decoder Control		 
  osThreadNew(tMotorControl, NULL, &thread_attr); 	// Motor Control Task  
	osThreadNew(tLED, NULL, NULL);										// LED Control Task
	osThreadNew(tmusicControl, NULL, NULL);						// Music Control Thread
	osKernelStart();                      						// Start thread execution
  
	for (;;) {
		//move(dir); for testing purposes comment out kernel and keep this
	}
}
