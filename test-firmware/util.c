#include "util.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

register int8_t wave asm("r2");
uint8_t (* volatile voiceptr)();
volatile uint8_t speed;

//https://stackoverflow.com/questions/1558321/how-do-i-generate-random-numbers-in-a-microcontroller-efficiently

uint8_t lfsr = 231;

uint8_t GetRandom()
{
	uint8_t input_bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 4)) & 1;
	lfsr = (lfsr >> 1) | (input_bit << 7);
	return lfsr;
}




ISR( TIMER0_COMPA_vect, ISR_NAKED  )
{
	if( wave < 0 )
	{
		//Switch the inverting rail.  WARNING: Do we need to put the second rail into high impedance mode when switching to prevent the moment where they're fighting each other?
	}
	else
	{
		//Switch the inverting rail.  WARNING: Do we need to put the second rail into high impedance mode when switching to prevent the moment where they're fighting each other?
		PORTB &=~_BV(3);
		PORTD &=~_BV(3);
	}
	asm( "reti" );
}

ISR( TIMER0_OVF_vect )
{
	if( wave < 0 )
	{
		PORTB |= _BV(3);
		PORTD |= _BV(3);
	}

	//Handle TIM0 OVR.  This happens at 31kHz.
	if( voiceptr() )
	{
			DDRB |= _BV(3);
			DDRD |= _BV(3);
	}
	else
	{
			DDRB &=~_BV(3);
			DDRD &=~_BV(3);
			OCR0A = OCR0B  = 255;  //XXX TODO see if 255 or 0 use less power.
			sei();
			return;
	}

	//Tricky: Need to almost immediately switch the value.
	OCR0A = OCR0B = ( ((uint8_t)wave)<<1);
	sei();

}


#define HAS_SAMPLES

#ifdef HAS_SAMPLES
const extern int8_t PROGMEM auddat[7981];
uint8_t voicePlayWave()
{
	static uint16_t sampleCount = 0;
	wave = pgm_read_byte( &auddat[sampleCount>>2] );
	if( sampleCount == 32000 )
	{
		voiceptr = voiceQuicklySleep;
		sampleCount = 0;
	}
	sampleCount++;
	return 1;
}
#else
uint8_t voicePlayWave()
{
	voiceptr = voiceQuicklySleep;
}

#endif

uint8_t voiceDoBasicSynth()
{
	int16_t twave;
	static uint8_t up;
	if( !speed )
	{
		voiceptr = voiceQuicklySleep;
		return 0;
	}

	twave = wave;
	if( up )
	{
		twave+=speed;
		if( twave > 100 ) { up = 0; twave = 100 - (twave-100); } //I hope this is right.
	}
	else
	{
		twave-=speed;
		if( twave < -100 ) { up = 1; twave = -100 - (twave+100); } //I hope this is right.
	}
	wave = twave;
	return 1;
}



uint8_t voiceQuicklySleep()
{
	return 0;
}
