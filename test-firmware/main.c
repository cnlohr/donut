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
		uint8_t i;
		i = PINB;
		if( !( i & _BV(0) ) ) voiceptr = &voicePlayWave;
		if( !( i & _BV(1) ) ) ts = 3;
		if( !( i & _BV(4) ) ) ts = 4; //SUSPECT
		if( !( i & _BV(5) ) ) ts = 16;
		i = PINC;
		if( !( i & _BV(0) ) ) ts = 5;
		if( !( i & _BV(1) ) ) ts = 6;
		if( !( i & _BV(2) ) ) ts = 17; // /SUSPECT
		if( !( i & _BV(3) ) ) ts = 8; // /SUSPECT
		if( !( i & _BV(5) ) ) ts = 9;
		if( !( i & _BV(4) ) ) ts = 16;
		i = PIND;
		if( !( i & _BV(0) ) ) ts = 10;
		if( !( i & _BV(2) ) ) ts = 11;
		if( !( i & _BV(4) ) ) ts = 12;
		if( !( i & _BV(7) ) ) ts = 13;
		i = PINE;
		if( !( i & _BV(1) ) ) ts = 18;
		if( !( i & _BV(2) ) ) ts = 14;
		if( !( i & _BV(3) ) ) ts = 15;
		speed = ts;
		if( ts != 0 )
		{
			voiceptr = &voiceDoBasicSynth;
			PORTD &=~_BV(1); //LED
		}
		else
		{
			PORTD |= _BV(1); //LED
		}
	}
}
