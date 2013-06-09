/* 
 * RC5 (36KHz Phillips protocol) Decoding library for AVR
 * Copyright (c) 2011 Filip Sobalski <pinkeen@gmail.com>
 * based on the idea presented by Guy Carpenter
 * on http://deep.clearwater.com.au/rc5/
 *
 * Version adapted to ATMega8 running on 8MHz internal oscillator 
 * 
 */
#ifndef RC5_H
#define RC5_H

#include <stdint.h>

#define RC5_GetStartBits(command) ((command & 0x3000) >> 12)
#define RC5_GetToggleBit(command) ((command & 0x800) >> 11)
#define RC5_GetAddressBits(command) ((command & 0x7C0) >> 6)
#define RC5_GetCommandBits(command) (command & 0x3F)
#define RC5_GetCommandAddressBits(command) (command & 0x7FF)

/* Initialize timer and interrupt */
void RC5_Init();

/* Reset the library back to waiting-for-start state */
void RC5_Reset();

/* Poll the library for new command.
 * 
 * You should call RC5_Reset immediately after
 * reading the new command because it's halted once 
 * receiving a full command to ensure you can read it
 * before it becomes overwritten. If you expect that only 
 * one remote at a time will be used then library
 * should be polled at least once per ~150ms to ensure
 * that no command is missed.
 */
uint8_t RC5_NewCommandReceived(uint16_t *new_command);


#endif

