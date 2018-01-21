#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "util.h"

register int8_t wave asm("r2");

uint8_t kit0_speed[] = {  23,  22,  21,  20,  19,  18,  17,  16,  15,  14,  13,  12,  11 };
uint16_t kit0_fade[] = {  24,  24,  24,  24,  24,  24,  24,  24,  24,  24,  24,  24,  24 };
uint8_t kit0_drop[] =  {  23,  22,  21,  20,  19,  18,  17,  16,  15,  14,  13,  12,  11 };

uint8_t kit1_speed[] = {  10,   9,   8,   7,   6,   5,   4,   3,   2,   2,   2,   2,   2 };
uint16_t kit1_fade[] = { 128, 512, 128, 128, 128, 128, 128, 128, 255, 255, 255, 512, 1024 };
uint8_t kit1_drop[] =  {  5,   64,  10,  10,  10,  16,  16,  16,  24,  24,  24,  64, 128 };

int main()
{
	CLKPR=0x80;
	CLKPR=0x01;

	//Set outputs to be: Speaker and TX light.
	DDRD = (_BV(3) | _BV(5) | _BV(6) | _BV(1) );
	DDRB = _BV(3);

	voiceptr = &voiceQuicklySleep;

//_BV(COM0A1) |
//_BV(COM0B1) | 
	TCCR0A = _BV(COM0B1) | _BV(COM0A1) | _BV(WGM01) | _BV(WGM00); //Phase correct PWM on TCCR0A, Clear on compare match.
	TCCR0B = _BV(CS00);
	TIMSK0 |= _BV(TOIE0);// | _BV(OCIE0A);
	OCR0A = OCR0B = 255;
	//DDRD &= ~_BV(5);
	//DDRD |= _BV(6);
	//DDRD &= ~_BV(6);



	PORTB |= _BV(0) | _BV(1) | _BV(4) | _BV(5);
	PORTC |= 0x3f;
	PORTD |= _BV(0) | _BV(2) | _BV(4) | _BV(7);
	PORTE |= _BV(3) | _BV(2) | _BV(1);

	sei();

	mode_button = 0;
	mode = 3;
	uint8_t sample0down = 0;
	uint8_t sample1down = 0;

	while(1)
	{
		uint8_t ts = 0;
		uint8_t ts1 = 0;

		uint8_t i;
		uint16_t checkmask = 1;

		if( MENUBUTTONDOWN )
		{
		 mode_button = 1;
		}
		else
		{
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

			if ( mode_button != 0) {
			  mode_button = 0;

			  voiceptr = voiceQuicklySleep;
			  speed = 0;
			  PORTD |= _BV(1);
			  mode = ts;
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
			    volume = 100;
			    volume1 = 100;

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
			    speed = ts;
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
			  }
			}
			else
			{
				//No notes
				voiceptr = voiceQuicklySleep;
				speed = 0;
				PORTD |= _BV(1); //LED
			}
		}
	}
}




volatile uint8_t speed1;
volatile uint8_t speed_rec1;
volatile uint8_t mode_button;
volatile uint8_t mode;
volatile uint16_t fade_out;
volatile uint8_t fade_out_mode;
