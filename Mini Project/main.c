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

volatile unsigned char rxData = 0x0;
volatile uint8_t dir = 0;
volatile uint8_t led_count = 0;

//  Prioritity of tBrain and tMotor
const osThreadAttr_t thread_attr = {
	.priority = osPriorityAboveNormal
};

osEventFlagsId_t data_flag;						// To Run tBrain when new data is received
osEventFlagsId_t dir_flag;						// To Run tControl after decoding of data
osEventFlagsId_t special_event_flag;	// For Connection and ending buzzer
osEventFlagsId_t init_light_flag;  		// Only raised when bluetooth start
osEventFlagsId_t init_audio_flag;  		// Only raised when bluetooth start


int songz[47] = {E4, E4, F4, G4, G4, F4, E4, D4, Z4, Z4, D4, E4, E4, D4, D4, E4, E4, F4, G4, G4, F4, E4, D4, Z4, Z4, D4, E4, D4, Z4, Z4, 
	D4, D4, E4, Z4, D4, E4, F4, E4, Z4, D4, E4, F4, E4, D4, Z4, D4, G3};

int connect_song[7] = {Z4, D4, E4, F4, G4, A5, B5};

int ending_song[18] = {A4, A4, A4, EF3, Z4, A4, EF3, Z4, A4, E4, E4, E4, F4, Z4, AB4, EF3, Z4, A4};

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
  osEventFlagsWait(init_light_flag, 0x0008, osFlagsWaitAny, osWaitForever);
	for(int i = 0; i<2; i++) {	
		PTC->PDOR |= MASK_GREEN_LED_ALL;
		osDelay(300);
		PTC->PDOR &= ~MASK_GREEN_LED_ALL;
		osDelay(300);
	}
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
	osEventFlagsWait(init_audio_flag, 0x0010, osFlagsWaitAny, osWaitForever);

	for (;;){
		for(int i=0; i < NORMAL_SONG_LENGTH; i+=1){
			if(rxData == 33) {
				osDelay(1000000);
			}
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
		if (rxData >> 5 == 1) {
			osEventFlagsSet(special_event_flag, 0x0004);
		}
		if (MASK_ON(rxData)) {
			dir |= (rxData & 0x1f);
		} else {
			dir &= ~(rxData & 0x1f);
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

void tSpecialEvent (void *argument) {
	//To control special events, e.g on connect or ending
	for (;;) {
		osEventFlagsWait(special_event_flag, 0x0004, osFlagsWaitAny, osWaitForever);
		osEventFlagsClear(special_event_flag, 0x0004);
		if(rxData == 32){
			initPWM();
			osEventFlagsSet(init_light_flag, 0x0008);
		// for just connected
			for(int i=0; i<CONNECT_SONG_LENGTH; i+=1){
				playSound(connect_song[i]);
				osDelay(NOTE_LENGTH);
				playSound(0);
				osDelay(BEAT);
			}
			osEventFlagsSet(init_audio_flag, 0x0010);

		} else if(rxData == 33) {
			for(int i=0; i<ENDING_SONG_LENGTH; i+=1){
				playSound(ending_song[i]);
				osDelay(NOTE_LENGTH);
				playSound(0);
				osDelay(BEAT);
			}
			SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
		}
	}
}
 
int main (void) {
 
  // System Initialization
  SystemCoreClockUpdate();
	initMotors();
	initUART2();
	initLED();
	
  // Initialize Kernel
  osKernelInitialize();                 						// Initialize CMSIS-RTOS
	dir_flag = osEventFlagsNew(NULL);
	data_flag = osEventFlagsNew(NULL);
	special_event_flag = osEventFlagsNew(NULL);
	init_light_flag = osEventFlagsNew(NULL);
	init_audio_flag = osEventFlagsNew(NULL);
  osThreadNew(tBrain, NULL, &thread_attr);    			// Decoder Control		 
  osThreadNew(tMotorControl, NULL, &thread_attr); 	// Motor Control Task  
	osThreadNew(tLED, NULL, NULL);										// LED Control Task
	osThreadNew(tmusicControl, NULL, NULL);						// Music Control Thread]
	osThreadNew(tSpecialEvent, NULL, &thread_attr);		// Music Control Thread
	osKernelStart();                      						// Start thread execution
  
	for (;;) {
		//move(dir); for testing purposes comment out kernel and keep this
	}
}
