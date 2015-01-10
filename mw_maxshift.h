/*
The MIT License (MIT)

Copyright (c) 2015 Solon Pissis and Ahmad Retha

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
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
