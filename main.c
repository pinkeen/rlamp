#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/delay_basic.h>
#include <avr/eeprom.h> 
#include <stdlib.h>

#include "bbpwm.h"
#include "rc5.h"

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a > b ? b : a)
#define SPEED_STEPS 6

typedef enum {S_OFF, S_WHITE, S_COLOR_WHEEL, S_PULSE, S_CANDLE, S_STORM, S_STROBO, S_NMAX, S_PROG, S_PROG_KEYS} State;
typedef enum {KEY_NEXT, KEY_PREV, KEY_FASTER, KEY_SLOWER, KEY_PAUSE, KEY_OFF, KEY_NONE} Key;

static uint16_t keys[KEY_NONE];
/* Used for saving key codes between power cycles*/
static uint16_t EEMEM eeprom_keys[KEY_NONE];

volatile static State state = S_OFF;
static uint8_t last_state;
static uint8_t paused = 0;

extern volatile uint8_t PWMDUTY[3];
/* We leech off of PWM counter to 
 * make a short non-blocking delay */
extern volatile uint8_t PWM_counter; 

/* Was key pressed? */
volatile static uint8_t key_pressed = 0;

/* Used for longer non-blocking delay */
volatile static uint8_t delay_counter = 0;


void setup()
{
    /* Set Timer2 prescaler to CLK/1024 .
     * At 8MHz it will be 128us per tick. */
    TCCR2 = _BV(CS20) | _BV(CS21) | _BV(CS22);
    /* Reset Timer2 counter */
    TCNT2 = 0;
    /* Enable Timer2 overflow interrupt */
    TIMSK |= _BV(TOIE2);
    
    /* Set PC0 to input */
    DDRC &= ~_BV(PC0);
    
    /* Read remote key codes.
     * It doesn't matter if we read garbage anyways
     * so no check */
    eeprom_read_block(keys, &eeprom_keys, 2 * KEY_NONE);
    
}

/* Triggers about once every 32ms */
ISR(TIMER2_OVF_vect)
{
    static uint8_t start = 0;
    static uint8_t ovfc = 0;
    delay_counter++;
    
    if(!(PINC & _BV(PINC0)))
    {
        ++ovfc;
        
        if(start)
        {
            if(ovfc >= 122)
            {
                /* Button was held for at least 4s.
                 * Enter remote programming mode. */
                last_state = state;
                state = S_PROG;
                
            }
            
            return;
        }
        
        start = 1;
        ovfc = 0;

        return;
    }
    
    
    if(start == 1 && ovfc >= 1)
        key_pressed = 1;

    start = 0;
}

void set_color(uint8_t r, uint8_t g, uint8_t b)
{
    PWMDUTY[0] = g;
    PWMDUTY[1] = b;
    PWMDUTY[2] = r;     
}

void set_color_idx(uint8_t idx, uint8_t amount)
{
    switch(idx)
    {
        case 0: set_color(amount, amount, amount); break;
        case 1: set_color(amount, 0, 0); break;
        case 2: set_color(0, amount, 0); break;
        case 3: set_color(0, 0, amount); break;
        case 4: set_color(amount, amount, 0); break;
        case 5: set_color(amount, 0, amount); break;
        case 6: set_color(0, amount, amount); break;
    }
}

/* Simple blocing delay, convenience function */
void delay(uint8_t i)
{
    while(--i) 
        _delay_loop_2(0);
}

/* Convenience function */
void flash_color(uint8_t r, uint8_t g, uint8_t b, uint8_t len)
{
    
    set_color(r, g, b);
    delay(len);
    set_color(0, 0, 0);
    delay(len);
}

/* This is used to filter out repeated commands generated
 * by remote when the key is being held. When key is being 
 * pressed repeatedly the toggle bit changes hence command
 * changes. */
uint8_t get_unique_command(uint16_t *command)
{
    uint16_t new_command;
    static uint16_t old_command = 0;
    
    if(!RC5_NewCommandReceived(&new_command))
        return 0;
    
    RC5_Reset();
    
    if(new_command == old_command)
        return 0;
    
    *command = new_command;
    old_command = new_command;
    
    return 1;
}

/* Convenience function. Implements a non-blocking
 * delay of length being a multiple of 32ms (T2 OVF) */
uint8_t delayed(uint8_t amount)
{
    static uint8_t delay_counter_last = 0;
    
    if((delay_counter != delay_counter_last) && !(delay_counter % amount))
    {
        delay_counter_last = delay_counter;
        return 1;
    }
    
    return 0;
}

int main()
{
    setup();
    BBPWM_Init();
    RC5_Init();
    
    sei();
    
    /* Used in key programming mode */
    uint8_t prog_key_nr = 0;
    
    /* Used in color fading */
    uint8_t color_step = 0;
    uint8_t color_amount = 0;
    
    uint8_t speed = SPEED_STEPS / 2;
    
    /* RC5 command received */
    uint16_t command;
    
    /* Used for finer control of the PWM
     * counter based delay */
    uint8_t slow_counter = 0;
    
    uint8_t color_idx = 4;
    
    for(;;)
    {
        Key key = KEY_NONE;
        
        if(state < S_NMAX)
        {
            if(key_pressed)
            {
                key_pressed = 0;
                state++;
                
                if(state == S_NMAX)
                    state = 0;
            }
            
            if(get_unique_command(&command))
            {
                uint8_t code = RC5_GetCommandAddressBits(command);
                
                uint8_t i = 0;
                for(i = 0; i < KEY_NONE; ++i)
                    if(code == keys[i])
                        key = i;
                    
                switch(key)
                {
                    case KEY_OFF:
                    {
                        if(state != S_OFF)
                        {
                            last_state = state;
                            state = S_OFF;
                            break;
                        }
                        
                        state = last_state;
                        
                        break;
                    }
                    
                    case KEY_NEXT:
                    {
                        state++;
                        
                        if(state == S_NMAX)
                            state = 1;
        
                        break;
                    }
                    
                    case KEY_PREV:
                    {
                        if(state <= 1)
                            state = S_NMAX;
                        
                        --state;
                        
                        break;
                    }

                    case KEY_FASTER:
                    {
                        if(speed < SPEED_STEPS)
                        {
                            ++speed;
                            flash_color(0, 255, 0, 3);
                        }
                        
                        break;
                    }

                    case KEY_SLOWER:
                    {
                        if(speed > 1) 
                        {
                            --speed;
                            flash_color(0, 255, 0, 3);
                        }
                            
                        
                        break;
                    }
                    
                    case KEY_PAUSE:
                    {
                        if(state == S_STROBO || state == S_PULSE)
                        {
                            color_idx++;
                            color_idx %= 7;
                            break;
                        }
                        
                        if(state == S_COLOR_WHEEL)
                        {
                            paused = !paused;
                            flash_color(0, 255, 0, 3);
                        }
                    }
                    
                    default:;
                }
            }
            
        }
        
        switch(state)
        {
            case S_PROG:
            {
                /* Flash leds to let the user know
                 * that programming started */
                
                flash_color(255, 0, 0, 5);
                flash_color(0, 255, 0, 5);
                flash_color(0, 0, 255, 5);
                flash_color(255, 255, 255, 5);

                state = S_PROG_KEYS;
                prog_key_nr = 0;
                
                /* Reset RC5 in case there is
                 * a not consumed command */
                RC5_Reset();
            
                
                break;
            }

            case S_PROG_KEYS:
            {
                if(get_unique_command(&command))
                {
                    keys[prog_key_nr] = RC5_GetCommandAddressBits(command);
                    prog_key_nr++;

                    if(prog_key_nr == KEY_NONE)
                    {
                        state = S_OFF;
                        key_pressed = 0;
                        
                        eeprom_write_block(keys, &eeprom_keys, 2 * KEY_NONE);
                        
                        flash_color(255, 255, 255, 2);
                        flash_color(255, 255, 255, 2);
                        RC5_Reset();
                        
                        break;
                    }
                    
                    flash_color(0, 255, 0, 5);
                    
                }
                break;
            }
            
            case S_WHITE:
            {
                set_color(255, 255, 255);
                break;
            }
            
            case S_COLOR_WHEEL:
            {
                if(PWM_counter == 0)
                {
                    ++slow_counter;
                    
                    uint8_t r, g, b;
                    
                    switch(color_step)
                    {
                        case 0:
                            r = 255; g = 0; b = color_amount; break;
                        case 1:
                            r = 255 - color_amount; g = 0; b = 255; break;
                        case 2:
                            r = 0; g = color_amount; b = 255; break;
                        case 3:
                            r = 0; g = 255; b = 255 - color_amount; break;
                        case 4:
                            r = color_amount; g = 255; b = 0; break;
                        case 5:
                            r = 255; g = 255 - color_amount; b = 0; break;
                            
                    }
                    
                    set_color(r, g, b);

                    if(slow_counter % ((SPEED_STEPS - speed) * 3 + 1) == 0)
                    {
                        if(!paused)
                        {
                            ++color_amount;
                            
                            if(color_amount == 0)
                            {
                                color_step++;
                                if(color_step == 6)
                                    color_step = 0;
                            }
                        }
                    }
                }
                
                break;
            }
            
            case S_PULSE:
            {
                if(PWM_counter == 0)
                {
                    static uint8_t pulse = 0;
                    static int8_t pulse_dir = 1;

                    ++slow_counter;
                    
                    if(slow_counter % ((SPEED_STEPS - speed) + 1) == 0)
                    {
                        set_color_idx(color_idx, pulse + 20);
                        
                        if(pulse == 235)
                            pulse_dir = -1;
                        
                        if(pulse == 0)
                            pulse_dir = 1;
                        
                        pulse += pulse_dir;
                        
                        
                    }
                }
                
                break;
            }
            
            case S_CANDLE:
            {
                if(delayed(2))
                {
                    uint8_t r = (rand() % 100) + 156;
                    uint8_t g = r - (rand() % 10) - 70;
                    set_color(r, g, 0);
                }
                
                break;
            }
            
            case S_STORM:
            {
                if(delayed(2))
                {
                    static uint8_t chance = 20;
                    uint8_t w = 0;
                    uint8_t b = rand() % 30 + 20;
                    uint8_t g = rand() % 20 + 20;
                    
                    if(rand() % 255 < chance)
                    {
                        w = rand() % 155 + 100;
                        chance += rand() % 15;
                        
                        if(chance < 10)
                            chance = 10;
                    }
                    
                    
                    set_color(w, MAX(w, MIN(g, b)), MAX(w, b));
                }
                
                break;
            }
            
            case S_STROBO:
            {
                if(delayed((SPEED_STEPS - speed) * 2 + 3))
                {
                    set_color_idx(color_idx, 255);
                    _delay_loop_2(2000);
                    set_color(0, 0, 0);
                }
                
                break;
            }

            case S_OFF: set_color(0, 0, 0); break;
            
            default:;
        }
    }   


    return 0;
}