#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>

void setup_adc() {
    ADMUX = (1 << REFS1) | (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    DIDR0 |= (1 << ADC5D) | (1 << ADC4D) | (1 << ADC3D);
}

void start_conversion() {
    ADCSRA |= (1 << ADSC);
}

uint16_t read_channel(uint8_t pin) {
    ADMUX &= 0xF0; //clear the last 4 bits
    ADMUX |= pin;
    start_conversion();
    while(bit_is_set(ADCSRA, ADSC));
    return ADC;
}

void set_up() {
    DDRB |= 0x6; //set bit PB1 and PB2 to output
    DDRD |= (1 << DDD6);
}

int main() {

    uint8_t increment_flag = 0; 
    
    setup_adc();
    set_up();

    //Set up red
    TCCR0A |= (1 << WGM00) | (1 << COM0A1) | (0 << COM0A0);
    OCR0A = 0x01;
    TIMSK0 |= (1 << OCIE0A);

    //set up blue
    TCCR1A |= (1 << WGM12) | (0 << WGM11) | (1 << WGM10) | (1 << COM1A1) | (0 << COM1A0);
    OCR1A = 0x01;
    TIMSK1 |= (1 << OCIE1A);
    
    //set up green
    TCCR1A |= (1 << COM1B1) | (0 << COM1B0); 
    OCR1B = 0x01;
    TIMSK1 |= (1 << OCIE1B);
    
    TCCR0B |= (0 << WGM02) | (0x5);
    TCCR1B |= (0 << WGM13) | (0x5);

    while(1) {
	//Our values can reach 1024 but output to pwm can only go to 255 so
	//we approximate 
	OCR0A = read_channel(0x05)/4;
	OCR1A = read_channel(0x04)/4;
	OCR1B = read_channel(0x03)/4;

    }

    return 0;    
}
