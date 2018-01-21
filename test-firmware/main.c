#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include "util.h"

register int8_t wave asm("r2");

uint8_t kit0_speed[] = {  23,  22,  21,  20,  19,  18,  17,  16,  15,  14,  13,  12,  11 };
uint16_t kit0_fade[] = {  24,  24,  24,  24,  24,  24,  24,  24,  24,  24,  24,  24,  24 };
uint8_t kit0_drop[] =  {  23,  22,  21,  20,  19,  18,  17,  16,  15,  14,  13,  12,  11 };

uint8_t kit1_speed[] = {  14,  13,   16,  11,  10,   9,   8,   7,   6,   5,   4,   2,   2 };
uint16_t kit1_fade[] = { 128, 512,    8, 128, 128, 128, 128, 128, 255, 255, 255, 512,1024 };
uint8_t kit1_drop[] =  {  5,   64,    4,  10,  10,  16,  16,  16,  24,  24,  24,  64, 128 };

uint8_t sqfreqs[] =    { 23,  22,  21,  20,  19,  18,  17,  16,  15,  14,  13,  12,  11};

void wdt_first(void) __attribute__((naked)) __attribute__((section(".init3")));

void wdt_first(void)
{
	MCUSR = 0; // clear reset flags
	wdt_disable();
	// http://www.atmel.com/webdoc/AVRLibcReferenceManual/FAQ_1faq_softreset.html
}

int main()
{
	cli();
	mode_button = 0;
	mode = eeprom_read_byte(0);

	CLKPR=0x80;
	CLKPR=0x01;

	//Set outputs to be: Speaker and TX light.
	DDRD = (_BV(3) | _BV(5) | _BV(6) | _BV(1) );
	DDRB = _BV(3);

	speed = 0;
	PORTD |= _BV(1) | _BV(3) | _BV(5) | _BV(6);
	voiceptr = &voiceQuicklySleep;

	TCCR0A = _BV(COM0B1) | _BV(COM0A1) | _BV(WGM01) | _BV(WGM00); //Phase correct PWM on TCCR0A, Clear on compare match.
	TCCR0B = _BV(CS00);
	TIMSK0 |= _BV(TOIE0);// | _BV(OCIE0A);
	OCR0A = OCR0B = 255;

	PORTB |= _BV(0) | _BV(1) | _BV(4) | _BV(5);
	PORTC |= 0x3f;
	PORTD |= _BV(0) | _BV(2) | _BV(4) | _BV(7);
	PORTE |= _BV(3) | _BV(2) | _BV(1);

	sei();

	if ( mode > 13 )
	{
		mode = 3;
		eeprom_write_byte(0, mode);
	}
	uint8_t sample0down = 0;
	uint8_t sample1down = 0;
	uint8_t newmode = 0;
	uint8_t old_mode_button = 0;
	
	while(1)
	{
		mode_button = MENUBUTTONDOWN;
		mode = mode % 14;
		uint8_t ts = 0;
		uint8_t ts1 = 0;

		uint8_t i;
		uint16_t checkmask = 1;

		uint16_t mask = ReadButtonMask();
		for( i = 0; i < 16; i++ )
		{
			if( mask & checkmask ) {
				if( ts ) 
					ts1 = i+1;
				else
					ts = i+1;
			}
			checkmask<<=1;
		}

		#define BASENOTE 0
		if ( ts || ts1 )
		{
		  wdt_reset();
		  if ( mode_button )
		  {
		    newmode = ts;
		  }
		}
		if ( old_mode_button && !mode_button ) {
		  wdt_reset();
		  voiceptr = voiceQuicklySleep;
		  speed = 0;
		  mode = newmode;
		  PORTD |= _BV(1);
		  eeprom_write_byte(0, mode);
		}
		else if( ts != 0 || mode == 7 || mode == 9 || mode == 10)
		{
		  switch (mode) {
		  case 0:
		  case 1:
		  case 2:
		  case 3:
		  case 4:
		  case 5:
		    volume = 127;
		    volume1 = 127;

		    speed = freq_s[ts-1+BASENOTE+12*mode];
		    speed_rec = freq_rs[ts-1+BASENOTE+12*mode];
		    if( ts1 )
		    {
			//2 notes.
			speed1 = freq_s[ts1-1+BASENOTE+12*mode];
			speed_rec1 = freq_rs[ts1-1+BASENOTE+12*mode];
		    }
		    else
		    {
			//1 note
			speed1 = 0;
		    }
		    voiceptr = &voiceDoBasicSynth;
		    PORTD &=~_BV(1); //LED
		    break;
		  case 6:
		    volume = 10*ts;
		    speed = 14-ts;
		    voiceptr = &voiceNoise;
		    PORTD &=~_BV(1); //LED
		    break;
		  case 7:
		    if ( (ts > 0 && ts < 7) || (ts1 > 0 && ts1 < 7) )
		    {
		      if ( !sample0down )
		      {
		        sample0down = 1;
		        sample0Count = 0;
		        wavedone = 0;
		      }
		    }
		    else
		    {
		      sample0down = 0;
		    }
		    if ( ts > 6 || ts1 > 6 )
		    {
		      if ( !sample1down )
		      {
		        sample1down = 1;
		        sample1Count = 0;
		        wavedone = 0;
		      }
		    }
		    else
		    {
		      sample1down = 0;
		    }
		    if ( wavedone )
		    {
		      voiceptr = &voiceQuicklySleep;
		      PORTD |=_BV(1); //LED
		    }
		    else
		    {
		      voiceptr = &voicePlayWave;
		      PORTD &=~_BV(1); //LED
		    }
		    break;
		  case 8:
		    //Tuned noise
		    volume = 100;
		    if (ts) {
		      speed = 24-ts;
		    }

		    voiceptr = &voiceTunedNoise;
		    PORTD &=~_BV(1);
		    break;
		  case 9:
		    //Drum Synths
		    if (ts) {
		      volume = 100;
		      
		      speed = kit0_speed[ts-1];
		      fade_out = kit0_fade[ts-1];
		      fade_out_mode = kit0_drop[ts-1];
		      PORTD &=~_BV(1);
		    } else {
		      PORTD |= _BV(1);
		    }

		    voiceptr = &voiceDrums;
		    break;
		  case 10:
		    //Drum Synths
		    if (ts) {
		      volume = 100;

		      speed = kit1_speed[ts-1];
		      fade_out = kit1_fade[ts-1];
		      fade_out_mode = kit1_drop[ts-1];
		      PORTD &=~_BV(1);
		    } else {
		      PORTD |= _BV(1);
		    }

		    voiceptr = &voiceDrums;
		    break;
		  case 11:
		    voiceptr = &voiceDoSquare;
		    speed = sqfreqs[ts-1];
		    PORTD &=~_BV(1);
		    if ( ts1 )
		    {
		    	speed1 = sqfreqs[ts1-1];
		    }
		    else
		    {
		    	speed1 = 0;
		    }
		    break;
		  case 12:
		    voiceptr = &voiceDoSquare;
		    PORTD &=~_BV(1);
		    speed = sqfreqs[ts-1]*2;
		    if ( ts1 )
		    {
		    	speed1 = sqfreqs[ts1-1]*2;
		    }
		    else
		    {
		    	speed1 = 0;
		    }
		    break;
		  case 13:
		    voiceptr = &voiceDoSquare;
		    PORTD &=~_BV(1);
		    speed = sqfreqs[ts-1]*4;
		    if ( ts1 )
		    {
		    	speed1 = sqfreqs[ts1-1]*4;
		    }
		    else
		    {
		    	speed1 = 0;
		    }
		    break;
		  }
		}
		else
		{
			//No notes
			voiceptr = voiceQuicklySleep;
			speed = 0;
			PORTD |= _BV(1); //LED
		}
		old_mode_button = mode_button;
	}
}




volatile uint8_t speed1;
volatile uint8_t speed_rec1;
volatile uint8_t mode_button;
volatile uint8_t mode;
volatile uint16_t fade_out;
volatile uint8_t fade_out_mode;
