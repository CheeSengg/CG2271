/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/
 
#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
#include "motors.h"
#include "uart.h"
#include "sound.h"
#include "led.h"

#define MASK(x) (1 << (x)) 	

int songz[47] = {E4, E4, F4, G4, G4, F4, E4, D4, Z4, Z4, D4, E4, E4, D4, D4, E4, E4, F4, G4, G4, F4, E4, D4, Z4, Z4, D4, E4, D4, Z4, Z4, 
	D4, D4, E4, Z4, D4, E4, F4, E4, Z4, D4, E4, F4, E4, D4, Z4, D4, G3};

int tracker[4] = {GREEN_LED_1, GREEN_LED_2, GREEN_LED_4, GREEN_LED_5};

int redState = 0;
int movementState = 0;

osSemaphoreId_t movementSem;

//----------------------------- FOR LED ------------------------


//----------------------------- END LED -------------------------
unsigned char rxData = 0x0;
uint8_t volatile dir = 0;

const osThreadAttr_t thread_attr = {
	.priority = osPriorityAboveNormal
};

osEventFlagsId_t data_flag;
osEventFlagsId_t dir_flag;


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
void tBrain (void *argument) {
	// Decodes messages received from Serial Communication
	for(;;) {
		osEventFlagsWait(data_flag, 0x0002, osFlagsWaitAny, osWaitForever);
		
		osSemaphoreAcquire(movementSem, osWaitForever);
		if (MASK_ON(rxData)) {
			dir |= rxData;
			movementState = 1;
		} else {
			dir &= ~rxData;
			movementState = 0;
		}
		osSemaphoreRelease(movementSem);
		
		// can add more for like buzzers
		osEventFlagsSet(dir_flag, 0x0001);
	}
}

void tMotorControl (void *argument) {
  // Controls direction of the Motors
  for (;;) {
		osEventFlagsWait(dir_flag, 0x0001, osFlagsWaitAny, osWaitForever); 		// Waits for dir to be updated by tBrain before it goes into READY state
		move(dir);
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

void tGreenLEDControl(void *argument){
	for(;;){
		osSemaphoreAcquire(movementSem, osWaitForever);
		if(movementState){
		for(int a=0; a<4; a++){
			PTA->PDOR &= ~(MASK(GREEN_LED_1) | MASK(GREEN_LED_2) | MASK(GREEN_LED_4) | MASK(GREEN_LED_5));
			PTA->PDOR |= MASK(tracker[a]);
			osDelay(250);
			if(!movementState){a=4;}
			}
		} else {
		PTA->PDOR |= (MASK(GREEN_LED_1) | MASK(GREEN_LED_2) | MASK(GREEN_LED_4) | MASK(GREEN_LED_5));
		}
	}
}

void tRedLEDControl(void *argument){
	for(;;){
		osSemaphoreAcquire(movementSem, osWaitForever);
		int redDelay = 0;
		if(movementState){ redDelay = 250; } else { redDelay = 500;}
		if(redState){
		PTE->PDOR |= MASK(RED_LED);
		redState = 0;
		} else {
		PTE->PDOR &= ~MASK(RED_LED);
		redState = 1;
		}
		osDelay(redDelay);
		osSemaphoreRelease(movementSem);
	}
}
 
int main (void) {
 
  // System Initialization
  SystemCoreClockUpdate();
	initMotors();
	initUART2();
	initPWM();
	initLED();
	movementSem = osSemaphoreNew(1,1, NULL);          //Semaphore
  // Initialize Kernal
  osKernelInitialize();                 						// Initialize CMSIS-RTOS
  osThreadNew(tBrain, NULL, &thread_attr);    			// Decoder Control		 
  osThreadNew(tMotorControl, NULL, &thread_attr); 	// Motor Control Task  
	osThreadNew(tmusicControl, NULL, &thread_attr);
	osThreadNew(tGreenLEDControl, NULL, &thread_attr);
	osThreadNew(tRedLEDControl, NULL, &thread_attr);
	osKernelStart();                      						// Start thread execution
  
	for (;;) {
		//move(dir); for testing purposes comment out kernel and keep this
	}
}
