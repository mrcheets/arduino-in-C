#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile int countdown = 5;
volatile uint16_t timer_toggle_count = 0; 

void setup_input() {
    ADMUX |= (1 << REFS0) | (1 << MUX2) | (1 << MUX0);
    ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    DIDR0 |= (1 << ADC5D);
}

void setup_output() {
    DDRB |= 0x01;
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
    TIMSK1 &= ~(1 << WGM12);
    TCCR1B &= ~(1 << CS12);
    TCCR1B &= ~(1 << CS10);
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
    //Set CTC mode
    TCCR1A = 0;
    TCCR1B = 0;
    TIMSK1 |= (1 << OCIE1A);
    sei();
    TCCR1B |= (1 << WGM12) | (1 << CS10);

}

void stop_pwm() {
    TIMSK1 &= ~(1 << OCIE1A);
}

int main() {

    setup_input();
    setup_output();

    DDRC |= (1 << DDC4);
    PORTC |= (1 << PC4);


    uint16_t sensor_value = read_channel(0x05);
    uint16_t sensor_low = 1023;
    uint16_t sensor_high = 0;
   
    start_timer();

    while(countdown > 0) { 
    	if(sensor_value > sensor_high) {
    	    sensor_high = sensor_value;
    	}
    	if(sensor_value < sensor_low) {
    	    sensor_low = sensor_value;
    	}
    }

    stop_timer();

    PORTC &=~(1 << PC4);

    start_pwm();

    while(1) {
	uint16_t input_value = read_channel(0x05);
	uint16_t pitch = simple_map(input_value, sensor_low, sensor_high, 50, 4000);

	OCR1A = F_CPU/ pitch/ 2 / 64 - 1; //input_value;
	timer_toggle_count = 2 * pitch * 20 / 1000;
	//_delay_ms(10);
    }

    return 0;
}

ISR(TIMER1_COMPA_vect) {
    if(countdown > -1) {
	--countdown;
    }

    if(timer_toggle_count > 0){
	PORTB ^=0x01;
	timer_toggle_count--;
    } else {
	if(countdown <= 0) stop_pwm();
	PORTB &= ~(1 << PB0);
    }
}
