#include "StairPWM.h"
#include <SoftPWM.h>

SOFTPWM_DEFINE_CHANNEL(0,  DDRD, PORTD, PORTD4); // Pin 4
SOFTPWM_DEFINE_CHANNEL(1,  DDRD, PORTD, PORTD3); // Pin 3
SOFTPWM_DEFINE_CHANNEL(2,  DDRD, PORTD, PORTD2); // Pin 2
SOFTPWM_DEFINE_CHANNEL(3,  DDRC, PORTC, PORTC5); // Pin A5
SOFTPWM_DEFINE_CHANNEL(4,  DDRC, PORTC, PORTC4); // Pin A4
SOFTPWM_DEFINE_CHANNEL(5,  DDRC, PORTC, PORTC3); // Pin A3
SOFTPWM_DEFINE_CHANNEL(6,  DDRC, PORTC, PORTC2); // Pin A2
SOFTPWM_DEFINE_CHANNEL(7,  DDRC, PORTC, PORTC1); // Pin A1
SOFTPWM_DEFINE_CHANNEL(8,  DDRD, PORTD, PORTD5); // Pin 5
SOFTPWM_DEFINE_CHANNEL(9,  DDRD, PORTD, PORTD6); // Pin 6
SOFTPWM_DEFINE_CHANNEL(10, DDRD, PORTD, PORTD7); // Pin 7
SOFTPWM_DEFINE_CHANNEL(11, DDRB, PORTB, PORTB0); // Pin 8
SOFTPWM_DEFINE_CHANNEL(12, DDRB, PORTB, PORTB1); // Pin 9
SOFTPWM_DEFINE_CHANNEL(13, DDRB, PORTB, PORTB2); // Pin 10
SOFTPWM_DEFINE_CHANNEL(14, DDRB, PORTB, PORTB3); // Pin 11
SOFTPWM_DEFINE_CHANNEL(15, DDRB, PORTB, PORTB4); // Pin 12

SOFTPWM_DEFINE_OBJECT_WITH_PWM_LEVELS(STAIRS_PWM_CHANNELS, PWM_MAX_VALUE+1);

void StairPWM::begin()
{
    Palatis::SoftPWM.begin(400);
}

void StairPWM::set(uint8_t channel, PWM_INT value)
{
    if (channel >= STAIRS_PWM_CHANNELS)
        return;

    
    pwmValues[channel] = value;
    Palatis::SoftPWM.set(channel, value);
}

PWM_INT StairPWM::get(uint8_t channel) const
{
    if (channel >= STAIRS_PWM_CHANNELS)
        return 0;

    return pwmValues[channel];
}

void StairPWM::update(uint8_t channel, uint8_t phase)
{
    // channels[channel].update(phase);
}
