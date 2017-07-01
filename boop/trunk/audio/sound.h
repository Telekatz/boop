/*
    sound.h - sound core routines
    Copyright (C) 2007  Ch. Klippel <ck@mamalala.net>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SOUND_H
#define SOUND_H

#define SOUND_OFF	0x00
#define SOUND_ON	0x01

//IO PINS on P0
#define SND_ON		6
#define SND_EN		11
#define SND_PWM		7

void startSoundIRQ(void);

void initSound(void);
void switchSound(unsigned char onoff);
void playSound(unsigned char *sound, unsigned long len);
void stopSound(void);
void waitSound(void);

#endif
