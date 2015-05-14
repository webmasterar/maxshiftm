/*
 * MaxShiftM
 * Copyright (C) 2015 Solon Pissis and Ahmad Retha
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * Define WORD - uses all bits available in a computer word
 */
typedef unsigned long int WORD;

/**
 * WORD size
 */
#define BYTE_SIZE 8
#define WORD_SIZE sizeof ( WORD ) * BYTE_SIZE

/**
 * A structure to hold number of WORDs and bits needed to store m. Also contains
 * yWord mask to be used to clear the left-most bits on the most significant
 * WORD in the errors array
 */
typedef struct Limit
{
    unsigned int words;
    unsigned int h;
    unsigned int yIndex;
    WORD yWord;
} Limit;

/**
 * Function definitions
 */
#define max(a,b) ((a) > (b)) ? (a) : (b)
#define min(a,b) ((a) < (b)) ? (a) : (b)
Limit init_limit ( unsigned int h, struct Limit lim );
unsigned int popcount_words ( WORD * words, int length );
WORD * shift_words ( WORD * words, int length );
WORD * shiftc_words ( WORD * words, struct Limit lim );
WORD delta ( char a, char b );
void displayMatrix ( WORD *** M, unsigned int m, unsigned int n, unsigned int o );
unsigned int maxshiftm_hd ( unsigned char * p, unsigned int m, unsigned  char * t, unsigned int n, unsigned int h, unsigned int * ii, unsigned int * jj, unsigned int * dd );
unsigned int maxshiftm_hd_ls ( unsigned char * p, unsigned int m, unsigned  char * t, unsigned int n, unsigned int h, unsigned int * ii, unsigned int * jj, unsigned int * dd );

