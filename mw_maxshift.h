/*
 * MW-MaxShift
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
 * Define modes: Edit Distance and Hamming Distance
 */
#define MODE_ED (unsigned int) 0
#define MODE_HD (unsigned int) 1

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
 * A special structure for the Edit Distance model to hold both errors and the
 * factor length so more errors can be stored when deletions are counted
 */
typedef struct xWord
{
    WORD * errors;
    unsigned int h;
} xWord;

/*
 * Function definitions
 */
#define max(a,b) ((a) > (b)) ? (a) : (b)
#define min(a,b) ((a) < (b)) ? (a) : (b)
Limit init_limit ( unsigned int h, unsigned int mode, struct Limit lim );
unsigned int popcount_words ( WORD *words, int length );
WORD * shift_words ( WORD *words, int length );
Limit extend_edx_limit ( Limit lim, unsigned int times );
WORD * shiftc_words ( WORD *words, struct Limit lim );
xWord * shiftc_edx_words ( xWord *e, Limit lim );
WORD * bitminmax_words ( WORD *a, WORD *b, WORD *c, unsigned int length );
xWord * bitminmax_edx_words ( xWord *a, xWord *b, xWord *c, unsigned int length );
WORD * bitminmax_numerical_words ( WORD *a, WORD *b , unsigned int length );
WORD delta ( char a, char b );
void displayMatrix ( WORD ***M, unsigned int m, unsigned int n, unsigned int o );
void display_edx_matrix ( xWord ***M, unsigned int m, unsigned int n, unsigned int o );
unsigned int mw_maxshift_ed ( unsigned char * p, unsigned int m, unsigned  char * t, unsigned int n, unsigned int h, unsigned int *ii, unsigned int *jj, unsigned int *dd );
unsigned int mw_maxshift_ed_ls ( unsigned char * p, unsigned int m, unsigned  char * t, unsigned int n, unsigned int h, unsigned int *ii, unsigned int *jj, unsigned int *dd );
unsigned int mw_maxshift_hd ( unsigned char * p, unsigned int m, unsigned  char * t, unsigned int n, unsigned int h, unsigned int *ii, unsigned int *jj, unsigned int *dd );
unsigned int mw_maxshift_hd_ls ( unsigned char * p, unsigned int m, unsigned  char * t, unsigned int n, unsigned int h, unsigned int *ii, unsigned int *jj, unsigned int *dd );
