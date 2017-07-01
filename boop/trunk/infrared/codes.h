/*
    codes.h - structs for accessing the infrared-codetables
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

#ifndef CODES_H
#define CODES_H

typedef struct CODE_TABLE_C
{
	const unsigned char codes[42];
	const char *name;
} CODE_TABLE_C;

typedef struct CODE_TABLE_S
{
	const unsigned short codes[42];
	const char *name;
} CODE_TABLE_S;

typedef struct CODE_TABLE_I
{
	const unsigned int codes[42];
	const char *name;
} CODE_TABLE_I;

typedef struct CODE_TABLE_N
{
	const char *name;
} CODE_TABLE_N;

typedef struct CODE_TABLE_L
{	
	const unsigned long	pre_data;			// data which the remote sends before actual keycode
	const unsigned long	post_data;          // data which the remote sends after actual keycode
	const unsigned long long	toggle_bit_mask;	// toggle_bit
	const unsigned long	gap;				// time between signals
	const unsigned long	repeat_gap;         // time between two repeat codes
	const unsigned char	bits;				// bits (length of code)
	const unsigned char	pre_data_bits;		// length of pre_data
	const unsigned char	post_data_bits;		// length of post_data
	const unsigned char	rc6_bit;			// doubles signal length of this bit (only used for RC-6)
	const unsigned long	freq;				// modulation frequency
	const unsigned short	flags;				// flags
	const unsigned short	phead,shead;		// header
	const unsigned short	pthree,sthree;		// 3 (only used for RC-MM)
	const unsigned short	ptwo,stwo;			// 2 (only used for RC-MM)
	const unsigned short	pone,sone;			// 1
	const unsigned short	pzero,szero;		// 0
	const unsigned short	plead;				// leading pulse
	const unsigned short	ptrail;				// trailing pulse
	const unsigned short	pfoot,sfoot;		// foot
	const unsigned short	pre_p,pre_s;        // signal between pre_data and keycode
	const unsigned short	post_p, post_s;     // signal between keycode and post_code
	const unsigned short	prepeat,srepeat;	// indicate repeating
	const unsigned char	min_repeat;			// code is repeated at least x times code sent once -> min_repeat=0
	const unsigned char	duty_cycle;			// 0<duty cycle<=100 
	
	const unsigned int codes[42];
	const char *name;
	
}CODE_TABLE_L;



typedef struct TABLES_C
{
	const unsigned int		num_tables;
	const struct CODE_TABLE_C	table[];
} TABLES_C;

typedef struct TABLES_S
{
	const unsigned int		num_tables;
	const struct CODE_TABLE_S	table[];
} TABLES_S;

typedef struct TABLES_I
{
	const unsigned int		num_tables;
	const struct CODE_TABLE_I	table[];
} TABLES_I;

typedef struct TABLES_N
{
	const unsigned int		num_tables;
	const struct CODE_TABLE_N	table[];
} TABLES_N;

typedef struct TABLES_L
{
	const unsigned int		num_tables;
	const struct CODE_TABLE_L	table[];
} TABLES_L;

extern const struct ENCODERS encoders;

#endif
