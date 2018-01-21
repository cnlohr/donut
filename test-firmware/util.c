#include "util.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "calced_speeds.c"

int8_t wave;

uint8_t (* volatile voiceptr)();
volatile uint8_t speed;
volatile uint8_t speed_rec;
volatile uint8_t speed1;
volatile uint8_t speed_rec1;
volatile uint8_t volume;
volatile uint8_t volume1;
volatile uint16_t frametimer;
volatile uint8_t mode;
volatile uint8_t mode_button;
volatile uint16_t fade_in;
volatile uint16_t fade_out;
volatile uint8_t fade_in_mode;
volatile uint8_t fade_out_mode;

uint16_t GetFrametimer() //Make sure it gets frametimer as an atomic operation.
{
	uint16_t ret;
	cli();
	ret = frametimer;
	sei();
	return ret;
}

uint16_t lfsr = 0xACE1u;  /* Any nonzero start state will work. (from wikipedia) */

uint8_t GetRandom()
{
	/* taps: 16 14 13 11; feedback polynomial: x^16 + x^14 + x^13 + x^11 + 1 */
	uint8_t   bit  = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
	lfsr = (lfsr >> 1) | (bit << 15);
	return lfsr;
}




ISR( TIMER0_COMPA_vect, ISR_NAKED  )
{
/*	if( wave < 0 )
	{
		//Switch the inverting rail.  WARNING: Do we need to put the second rail into high impedance mode when switching to prevent the moment where they're fighting each other?
	}
	else
	{
		//Switch the inverting rail.  WARNING: Do we need to put the second rail into high impedance mode when switching to prevent the moment where they're fighting each other?
*/
//	}
	asm( "reti" );
}


//		mov r25, r2\n\
		lsl r2\n\
		out 0x27,r2 /*OCR0A*/ \n\
		out 0x28,r2 /*OCR0B*/ \n\
		brcs cont__\n\
		cbi 0xa,3 /*DDRD |= _BV(3) */\n\
		cbi 0x4,3 /*DDRB |= _BV(3) */\n\
cont__:\n\
		push r0\n\

//#define NAKED_T0OVF

ISR( TIMER0_OVF_vect, ISR_NAKED )
{
	asm( "\n\
		sbis 0x1E, 1	/* If we aren't supposed to do any operations, don't skip the jump to normal operation */\n\
		rjmp intcont\n\
		sbic 0x1E, 0	/* If _BV(0) is set, continue to the below code. */ \n\
		rjmp make_zero\n\
		sbi 0xb,3 /*PORT |= _BV(3) */\n\
		sbi 0x5,3 /*PORTB |= _BV(3) */\n\
		rjmp intcont\n\
make_zero:\n\
		cbi 0xb,3 /*PORTD &=~_BV(3) */\n\
		cbi 0x5,3 /*PORTB &=~_BV(3) */\n\
intcont:\n\
		push r0\n\
		push r1\n\
		in r0,63 /*Store SREG*/\n\
		push r0\n\
		clr r1\n\
		push r16\n\
		push r17\n\
		push r18\n\
		push r19\n\
		push r20\n\
		push r21\n\
		push r22\n\
		push r23\n\
		push r24\n\
		push r25\n\
		push r30\n\
		push r31\n\
	");


	frametimer++;
	/*Ok... We can do whatever we want in here, as long as it's fast and before the end we update
		OCR0A and OCR0B.  That will cause the actual registers to update AFTER the next cycle. */

	//Handle TIM0 OVR.  This happens at 31kHz.
	if( !voiceptr() )
	{
			//This disables the output drive.
			DDRB &=~_BV(3);
			DDRD &=~_BV(3);
			PORTB |= _BV(3);
			PORTD |= _BV(3);
			OCR0A = OCR0B = 255;
			goto end_int;
	}
	else
	{
		DDRD |= _BV(3);
		DDRB |= _BV(3);
	}

	//Tricky: Detect if we're switching states, if so, need to trigger early-interrupt-handler transition of states.
	//This data is stored in GPIOR0 so we can operate at the beginning of the next interrupt without issues.
	if( wave < 0 )
	{
		if( GPIOR0 & _BV(0) )
			GPIOR0 |= _BV(1); //If transition, trigger.
		else
			GPIOR0 &=~_BV(1);
		GPIOR0 &= ~_BV(0);
	}
	else
	{
		if( !(GPIOR0 & _BV(0)) )
			GPIOR0 |= _BV(1); //If transition, trigger.
		else
			GPIOR0 &=~_BV(1);
		GPIOR0 |= _BV(0);
	}

	OCR0A = OCR0B = ((uint8_t)wave)<<1;
end_int:
	asm( "\
		pop r31\n\
		pop r30\n\
		pop r25\n\
		pop r24\n\
		pop r23\n\
		pop r22\n\
		pop r21\n\
		pop r20\n\
		pop r19\n\
		pop r18\n\
		pop r17\n\
		pop r16\n\
		pop r0\n\
		out 63, r0 /*Restore SREG*/\n\
		pop r1\n\
		pop r0\n\
		reti\n");
}


#define HAS_SAMPLES
#ifdef HAS_SAMPLES
#include "samples.c"
const extern int8_t PROGMEM auddat[NUM_SAMPLES];
uint16_t sampleCount = 0;
uint8_t voicePlayWave()
{
	wave = pgm_read_byte( &auddat[sampleCount] );
	if( sampleCount == NUM_SAMPLES )
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
	static uint8_t up1;
	static int8_t wave0;
	static int8_t wave1;


	if( !speed )
	{
		voiceptr = voiceQuicklySleep;
		wave0 = wave1 = 0;
		return 0;
	}

	twave = wave0;
	if( up )
	{
		twave+=speed;
		if( twave > speed_rec ) { up = 0; twave = speed_rec - (twave-speed_rec); } //I hope this is right.
	}
	else
	{
		twave-=speed;
		if( twave < -speed_rec ) { up = 1; twave = -speed_rec - (twave+speed_rec); } //I hope this is right.
	}
	wave0 = twave;

	if( speed1 )
	{
		twave = wave1;
		if( up1 )
		{
			twave+=speed1;
			if( twave > speed_rec1 ) { up1 = 0; twave = speed_rec1 - (twave-speed_rec1); } //I hope this is right.
		}
		else
		{
			twave-=speed1;
			if( twave < -speed_rec1 ) { up1 = 1; twave = -speed_rec1 - (twave+speed_rec1); } //I hope this is right.
		}
		wave1 = twave;
		wave = ((wave1*volume)>>8)+((wave0*volume)>>8);
		return 1;
	}
	else
	{
		wave = (wave0*volume)>>8;
		wave1 = 0;
		return 1;
	}
}

uint8_t voiceTunedNoise()
{
  static uint8_t mark_s, mark_s1;

  mark_s++;
  
  if (mark_s == speed) {
    wave = (((int8_t)GetRandom()) * volume) >> 8;
    mark_s = 0;
  }

  return 1;
}

uint8_t voiceDrums()
{
  static uint8_t drum_s;
  static uint16_t ticks;

  drum_s++;

  if (drum_s == speed) {
    wave = (((int8_t)GetRandom()) * volume) >> 8;
    drum_s = 0;
    ticks++;

    if (ticks > fade_out) {
      volume--;
    }
  }

  if (volume == 0) {
    ticks = 0;
    drum_s = 0;
    return 0;
  }

  return 1;
}

uint8_t voiceQuicklySleep()
{
	return 0;
}

uint8_t voiceNoise()
{
	static uint8_t speedmark;
	speedmark++;

	if( speedmark == speed )
	{
		wave = (((int8_t)GetRandom()) * volume)>>8;
		speedmark = 0;
	}
	return 1;
}

uint16_t ReadButtonMask()
{
	uint16_t ret = 0;
	if( BUTTON0DOWN ) ret |= _BV(0);
	if( BUTTON1DOWN ) ret |= _BV(1);
	if( BUTTON2DOWN ) ret |= _BV(2);
	if( BUTTON3DOWN ) ret |= _BV(3);
	if( BUTTON4DOWN ) ret |= _BV(4);
	if( BUTTON5DOWN ) ret |= _BV(5);
	if( BUTTON6DOWN ) ret |= _BV(6);
	if( BUTTON7DOWN ) ret |= _BV(7);
	if( BUTTON8DOWN ) ret |= _BV(8);
	if( BUTTON9DOWN ) ret |= _BV(9);
	if( BUTTON10DOWN ) ret |= _BV(10);
	if( BUTTON11DOWN ) ret |= _BV(11);
	if( BUTTON12DOWN ) ret |= _BV(12);
	return ret;
}

