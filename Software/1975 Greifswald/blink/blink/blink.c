#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>

int
main (void)
{
	DDRA |= _BV(DDA1);
	
	while(1)
	{
		PORTB ^= _BV(PA1);
		_delay_ms(1000);
	}
}