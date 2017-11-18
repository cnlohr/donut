#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

uint8_t speed;
register int8_t wave asm("r2");
volatile uint16_t audmark;
extern const int8_t PROGMEM auddat[];

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

#if 0
	int16_t twave;
	static int8_t up;
	static uint8_t wave;
	if( up )
	{
		wave+=2;
		if( wave == 100 ) up = 0;
	}
	else
	{
		wave-=2;
		if( wave == 0 ) up = 1;
	}
	OCR0A = OCR0B = wave;
		PORTB |= _BV(3);
		PORTD |= _BV(3);

#else

	if( audmark )
	{
		wave = pgm_read_byte( &auddat[audmark>>2] );
		audmark++;
		DDRB |= _BV(3);
		DDRD |= _BV(3);
		if( audmark == 32000 ) audmark = 0;
	}
	else
	{
		int16_t twave;
		static uint8_t up;

		if( speed == 0 )
		{
			DDRB &=~_BV(3);
			DDRD &=~_BV(3);
		}
		else
		{
			DDRB |= _BV(3);
			DDRD |= _BV(3);

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
	}

//	if( wave > 0 )
//		TIFR0 |= _BV(OCF0A);
	OCR0A = OCR0B = ( ((uint8_t)wave)<<1);

	sei();

#endif
}


int main()
{
	CLKPR=0x80;
	CLKPR=0x01;

	//Set outputs to be: Speaker and TX light.
	DDRD = (_BV(3) | _BV(5) | _BV(6) | _BV(1) );
	DDRB = _BV(3);

	DDRD &= ~_BV(6);

	TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00); //Phase correct PWM on TCCR0A, Clear on compare match.
	TCCR0B = _BV(CS00);
	TIMSK0 |= _BV(TOIE0) | _BV(OCIE0A);
	sei();


	PORTB |= _BV(0) | _BV(1) | _BV(4) | _BV(5);
	PORTC |= 0x3f;
	PORTD |= _BV(0) | _BV(2) | _BV(4) | _BV(7);
	PORTE |= _BV(3) | _BV(2) | _BV(1);

	while(1)
	{
		uint8_t ts = 0;
		uint8_t i;
		i = PINB;
		if( !( i & _BV(0) ) ) audmark = 1;
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
		if( ts != 0 || audmark)
		{
			PORTD &=~_BV(1); //LED
		}
		else
		{
			PORTD |= _BV(1); //LED
		}
		speed = ts;
	}
}
