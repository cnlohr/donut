#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "util.h"


register int8_t wave asm("r2");


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



	while(1)
	{
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

#define BEND 24

		if( ts != 0 )
		{
			speed = freq_s[ts-1+BEND];
			speed_rec = freq_rs[ts-1+BEND];
			if( ts1 )
			{
				speed1 = freq_s[ts1-1+BEND];
				speed_rec1 = freq_rs[ts1-1+BEND];
			}
			else
			{
				speed1 = 0;
			}
			voiceptr = &voiceDoBasicSynth;
			PORTD &=~_BV(1); //LED
		}
		else
		{
			speed = 0;
			PORTD |= _BV(1); //LED
		}
	}
}




volatile uint8_t speed1;
volatile uint8_t speed_rec1;
