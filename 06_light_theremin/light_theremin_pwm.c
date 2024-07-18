#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile uint32_t countdown = 0;
volatile uint16_t timer_toggle_count = 0; 

void setup_input() {
    ADMUX |= (1 << REFS0) | (1 << MUX2) | (1 << MUX0);
    ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    DIDR0 |= (1 << ADC5D);
}

void setup_output() {
    DDRB |= 0x02;
}

/**
 * Set this to throw an interrupt every second.
 */
void start_timer() {
    TCCR1B |= (1 << WGM12);
    TIMSK1 |= (1 << OCIE1A);
    sei();
    OCR1A = 15625;
    TCCR1B |= (1 << CS12) | (1 << CS10);
}

void stop_timer() {
    TCCR1B &= ~(1 << WGM12);
    TIMSK1 &= ~(1 << OCIE1A);
    TCCR1B &= ~(1 << CS12);
    TCCR1B &= ~(1 << CS10);
    cli();
}

void start_conversion() {
    ADCSRA |= (1 << ADSC);
}

uint16_t read_channel(uint8_t channel) {
    ADMUX &= 0xF0;
    ADMUX |= channel;
    start_conversion();
    while(bit_is_set(ADCSRA, ADSC));
    return ADC;
}

uint16_t simple_map(uint16_t value, uint16_t from_low, uint16_t from_high, uint16_t to_low, uint16_t to_high) {
    uint16_t ratio = (to_high - to_low)/(from_high - from_low);
    return (value - from_low) * ratio + to_low;
}

void start_pwm() {
    TCCR1A |= (1 << WGM11) | (1 << COM1A1);
    TCCR1B = 0;
    TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS10);

}

void stop_pwm() {
    TIMSK1 &= ~(1 << OCIE1A);
}

int main() {

    setup_input();
    setup_output();

    DDRC |= (1 << DDC4);
    PORTC |= (1 << PC4);


    uint16_t sensor_value = 0;
    uint16_t sensor_low = 1023;
    uint16_t sensor_high = 0;
    uint16_t input_value = 0;
    uint16_t top = 0;
    
    //This uses interrupts. For some reason, counting down with interrupts introduces glitches in the final pwm
    start_timer();

    //Give it time to set the bits otherwise it won't set correctly
    _delay_ms(50);

    while(countdown < 5) {
    	sensor_value = read_channel(0x05); 
    	if(sensor_value > sensor_high) {
    	    sensor_high = sensor_value;
    	}
    	if(sensor_value < sensor_low) {
    	    sensor_low = sensor_value;
    	}
	//countdown++;
    }

    stop_timer();
    _delay_ms(50);

    PORTC &=~(1 << PC4);
    _delay_ms(50);

    start_pwm();
    
    if(sensor_low == 1023 || (sensor_low >= sensor_high)) sensor_low = 0;
    if(sensor_high == 0 || (sensor_high <= sensor_low)) sensor_high = 1023;
        
    ICR1 = 2500;
    top = 1000;
    while(1) {
        top = simple_map(read_channel(0x05), 0, 1023, 2000, 65000);
    	if(top != ICR1) {
    	    ICR1 = top;
    	    OCR1A = top/2; 
    	}
    	
    	_delay_ms(20);
    }

    return 0;
}

ISR(TIMER1_COMPA_vect) {
    ++countdown;
}
