#ifndef GREENLED_H_
#define GREENLED_H_

// GREEN LED pins
#define GREEN_LED 0

// Moving LED delay
#define G_MOVING_DELAY	1000	// 1000 ms

void initGreenLed(void);

void greenStationary(void);

void greenMoving(void);

#endif
