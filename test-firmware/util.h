#ifndef _UTIL_H
#define _UTIL_H

#include <stdint.h>

extern uint16_t lfsr; //last value
uint8_t GetRandom();

//extern register int8_t wave asm("r2");
extern uint8_t (*volatile voiceptr)();  //Your function should update "wave" and return 0 if silent, and nonzero if making noise.
extern volatile uint8_t speed;  //Use this as the "pitch" or code for your first instrument.  Assume 0 makes instrument quiet.
extern volatile uint8_t speed_rec;
extern volatile uint8_t speed1;  //This is effectively a second voice.  Cannot be used if voice0 is off.
extern volatile uint8_t speed_rec1;
extern volatile uint8_t volume;
extern volatile uint8_t volume1; //Adjust volume.  CAREFUL: If you are playing two notes, the sum of the volume MUST NOT EXCCED 255.
extern volatile uint16_t sample0Count;
extern volatile uint16_t sample1Count;
extern volatile uint8_t wavedone;

extern volatile uint16_t frametimer; //Incremented every cycle.
extern volatile uint8_t mode; //Current mode
extern volatile uint8_t mode_button; //Mode button depressed
extern volatile uint16_t fade_out;
extern volatile uint8_t fade_out_mode;

uint16_t GetFrametimer(); //Make sure it gets frametimer as an atomic operation.


	//Actual frequency is computed by:
	//  31250 / speed_rec * speed
	//  requirement: speed <= speed_rec <= 127


//Useful table for calculating good speed/recropricals for times.
extern uint8_t freq_s[];
extern uint8_t freq_rs[];


uint8_t voiceQuicklySleep();
uint8_t voicePlayWave();
uint8_t voiceDoBasicSynth();
uint8_t voiceNoise();
uint8_t voiceTunedNoise();
uint8_t voiceDoSquare();
uint8_t voiceDrums();


uint16_t ReadButtonMask(); //Only reads musical notes.  For the menu button, must look at it directly.

#define BUTTON0DOWN		(!( PINC & _BV(5) ))
#define BUTTON1DOWN		(!( PIND & _BV(2) ))
#define BUTTON2DOWN		(!( PIND & _BV(4) ))
#define BUTTON3DOWN		(!( PINE & _BV(1) ))
#define BUTTON4DOWN		(!( PIND & _BV(7) ))
#define BUTTON5DOWN  	(!( PINB & _BV(0) ))
#define BUTTON6DOWN  	(!( PINB & _BV(1) ))
#define BUTTON7DOWN 	(!( PINB & _BV(4) ))
#define BUTTON8DOWN		(!( PINE & _BV(2) ))
#define BUTTON9DOWN		(!( PINE & _BV(3) ))
#define BUTTON10DOWN    (!( PINC & _BV(2) ))
#define BUTTON11DOWN    (!( PINC & _BV(3) ))
#define BUTTON12DOWN	(!( PINC & _BV(4) ))

#define MENUBUTTONDOWN  (!( PINB & _BV(5) ))
//PC0 = Touch1
//PC1 = Touch2
//PD0 = RX


#endif

