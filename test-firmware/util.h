#ifndef _UTIL_H
#define _UTIL_H

#include <stdint.h>

extern uint8_t lfsr; //last value
uint8_t GetRandom();

//extern register int8_t wave asm("r2");
extern uint8_t (*voiceptr)();  //Your function should update "wave" and return 0 if silent, and nonzero if making noise.
extern uint8_t speed;  //Use this as the "pitch" or code for your first instrument.  Assume 0 makes instrument quiet.

uint8_t voiceDoBasicSynth();


#endif

