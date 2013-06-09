#include "bbpwm.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define PWMDDR DDRB
#define PWMPORT PORTB

#define PWMPA _BV(PB1)
#define PWMPB _BV(PB2)
#define PWMPC _BV(PB3)

#define PWMPMASK (PWMPA | PWMPB | PWMPC)

volatile uint8_t PWMDUTY[3] = {0, 0, 0};
volatile uint8_t PWM_counter = 0xFF;

void BBPWM_Init()
{
    /* Set prescaler to CLK/1 */
    TCCR0 = _BV(CS00);

    /* Enable overflow interrupt */
    TIMSK |= _BV(TOIE0);
    
    /* Set ports to output */
    PWMDDR |= PWMPMASK;
    
    /* Set ports to LOW */
    PWMPORT &= ~PWMPMASK;
}

ISR(TIMER0_OVF_vect) 
{
    static uint8_t duty_buf[3];

    uint8_t mask_on = 0;
    uint8_t mask_off = 0;
        
    if(++PWM_counter == 0)
    {
        mask_on |= PWMPMASK;

        duty_buf[0] = PWMDUTY[0];
        duty_buf[1] = PWMDUTY[1];
        duty_buf[2] = PWMDUTY[2];
    }
    
    if(PWM_counter == duty_buf[0]) mask_off |= PWMPA;
    if(PWM_counter == duty_buf[1]) mask_off |= PWMPB;
    if(PWM_counter == duty_buf[2]) mask_off |= PWMPC;
    
    mask_on &= ~mask_off;
    PWMPORT |= mask_on;
    PWMPORT &= ~mask_off;
}