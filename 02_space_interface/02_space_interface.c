#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>

int main() {

    DDRB |= (1 << DDB0); 
    DDRD |= (1 << DDD7 | 1 << DDD6);
    DDRC &= ~(1 << DDC5);
    
    while (1) {
	
	PORTB |= (1 << PB0);
	
	if(0x01 & (PINC >> PINC5)) {
	    PORTD |= (1 << PD7 | 1 << PD6);
	    PORTB &= ~(1 << PB0);

	    _delay_ms(250);

	    PORTD &= ~(1 << PD7);
	    PORTD &= ~(1 << PD6);

	    _delay_ms(250);
	}
    }

    return 0;
}
   
