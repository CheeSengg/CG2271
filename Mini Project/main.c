/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/
 
#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
#include "motors.h"

#define UART_TX_PORTE22 22
#define UART_RX_PORTE23 23
#define BAUD_RATE 9600
#define UART2_INT_PRIO 128

unsigned char rxData = 0x0;
uint8_t volatile dir = 0;

const osThreadAttr_t thread_attr = {
	.priority = osPriorityAboveNormal
};

osEventFlagsId_t data_flag;
osEventFlagsId_t dir_flag;

/* Init UART2 */
void initUART2(uint32_t baud_rate)
{
	uint32_t divisor, bus_clock;
	SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
	
	PORTE->PCR[UART_TX_PORTE22] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[UART_TX_PORTE22] |= PORT_PCR_MUX(4);
	
	PORTE->PCR[UART_RX_PORTE23] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[UART_RX_PORTE23] |= PORT_PCR_MUX(4);

	UART2->C2 &= ~(UART_C2_RE_MASK);
	
	bus_clock = (DEFAULT_SYSTEM_CLOCK)/2;
	divisor = bus_clock / (baud_rate * 16);
	UART2->BDH = UART_BDH_SBR(divisor >> 8);
	UART2->BDL = UART_BDL_SBR(divisor);
	
	UART2->C1 = 0;
	UART2->S2 = 0;
	UART2->C3 = 0;
	
	UART2->C2 |= ( (UART_C2_RE_MASK));
	
	NVIC_SetPriority(UART2_IRQn, 128);
	NVIC_ClearPendingIRQ(UART2_IRQn);
	NVIC_EnableIRQ(UART2_IRQn);
	UART2->C2 |= UART_C2_RIE_MASK;
}


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
		move(dir);
	}
}
 
int main (void) {
 
  // System Initialization
  SystemCoreClockUpdate();
	initMotors();
	initUART2(BAUD_RATE);
	
  // Initialize Kernal
  osKernelInitialize();                 						// Initialize CMSIS-RTOS
  osThreadNew(tBrain, NULL, &thread_attr);    			// Decoder Control		 
  osThreadNew(tMotorControl, NULL, &thread_attr); 	// Motor Control Task  
	osKernelStart();                      						// Start thread execution
  
	for (;;) {
		//move(dir); for testing purposes comment out kernel and keep this
	}
}
