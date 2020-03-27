#ifndef REDLED_H_
#define REDLED_H_

// RED LED pins
#define RED_LED 0

// Delay for different modes
#define R_MOVING_DELAY				500		// 500 ms
#define R_STATIONARY_DELAY 		250		// 250 ms

void initRedLed(void);

void redStationary(void);

void redMoving(void);

#endif
