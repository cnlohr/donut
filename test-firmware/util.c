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
		pop r0\n\
		out 63, r0 /*Restore SREG*/\n\
		pop r1\n\
		pop r0\n\
		reti\n");
}



#ifdef HAS_SAMPLES
#include "samples.c"
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
		wave = (wave1+wave0)/2;
		return 1;
	}
	else
	{
		wave = wave0;
		wave1 = 0;
		return 1;
	}
}



uint8_t voiceQuicklySleep()
{
	return 0;
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

