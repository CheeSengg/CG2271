#ifndef SOUND_H_
#define SOUND_H_

#define PTB0_Pin 0
#define PTB1_Pin 1

#define NOTE_LENGTH 300
#define BEAT 70
#define EF3 892
#define G3 796
#define AB4 751
#define A4 710
#define B4 632
//#define C4 596
#define Z4 596
#define D4 531
#define E4 473
#define F4 447
#define G4 398
#define A5 355
#define B5 316
//#define C5 298
#define Z5 298
#define NORMAL_SONG_LENGTH 47
#define CONNECT_SONG_LENGTH 7
#define ENDING_SONG_LENGTH 18

void initPWM(void);
void playSound(int);

#endif
