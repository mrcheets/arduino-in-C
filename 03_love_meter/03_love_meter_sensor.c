#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void start_conversion();
uint16_t duty_cycle = 0;
volatile static uint8_t adc_convert_done = 0;

void setup_adc() {
    ADMUX = (1 << REFS0) | (1 << MUX2) | (1 << MUX0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    DIDR0 |= (1 << ADC5D);
}

void start_conversion() {
    ADCSRA |= (1 << ADSC);
}

void stop_conversion() {
    ADCSRA &= ~(1 << ADSC);
}

int main() {

    const float baseline_temp = 20.0;

    DDRB |= (1 << DDB0); 
    DDRD |= (1 << DDD7 | 1 << DDD6);
    setup_adc();

    while(1) {
    
    	start_conversion();
    	
    	while(bit_is_set(ADCSRA, ADSC));
    	
    	duty_cycle = ADC; //use 16bit variable and get ADC rather than ADCH + ADCL

	float voltage = (duty_cycle/1024.0) * 5.0;
	float temp = (voltage - .5) * 100;

	if(temp < baseline_temp + 2) {
	    PORTB &= ~(1 << PB0);
	    PORTD &= ~(1 << PD7);
	    PORTD &= ~(1 << PD6);
	} else if (temp >= baseline_temp + 2 && temp < baseline_temp + 4) {
	    PORTB |= (1 << PB0);
	    PORTD &= ~(1 << PD7);
	    PORTD &= ~(1 << PD6);
	} else if (temp >= baseline_temp + 4 && temp < baseline_temp + 6) {
	    PORTB |= (1 << PB0);
	    PORTD |= (1 << PD7);
	    PORTD &= ~(1 << PD6);
	} else if (temp >= baseline_temp + 6) {
	    PORTB |= (1 << PB0);
	    PORTD |= (1 << PD7);
	    PORTD |= (1 << PD6);
	}
    }

    return 0;
}

