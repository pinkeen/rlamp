# Remote-controlled RGB lamp

## What is it?

Source, schematics and boards for a remote controlled RGB lamp based on AVR.

Everything you need to build one yourself should be here.

### So, what does it do?

It lights up. It shines pretty colours. 

You can use any RC5 (90% of household remotes) compatible remote to control it.

The commands are programmable, you can bind any key to any command.

## How to use it?

As far as I remember it has a few modes, some of them can be adjusted somehow:

- White
- Color wheel (adjustable speed), can be paused on any colour.
- Candle
- Storm
- Pulse (you can change the colour, I think...)
- Strobo (adjustable something)
- Off (peace, at last)

## The docs

Uh, oh... I don't remember. Will try to add some soon along with pics or maybe a video.

I attached the schematics of the main board as pdf. The schematics and boards are made in *KiCAD*.

The second board is just a bed for a star led. You will probably design your own to fit your led.
The two boards should form a sandwich...

The lfuse.bin file contains fuse settings, which I probably... ehhh... fused... for... Sorry, don't remember, you
have to check it in uC's (ATmega88?) the datasheet.

## Licenses

### The application (source code)

rlamp - Remote controlled rgb lamp based on AVR
Copyright &copy; 2011 Filip Sobalski <pinkeen@gmail.com>

This file is part of rlamp.

Rlamp is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

### The boards and schematics

The boards and schematics are licensed under a [Creative Commons Attribution 3.0 Unported License](http://creativecommons.org/licenses/by/3.0/).


[![Bitdeli Badge](https://d2weczhvl823v0.cloudfront.net/pinkeen/rlamp/trend.png)](https://bitdeli.com/free "Bitdeli Badge")

