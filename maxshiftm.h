/*
 * MaxShiftM
 * Copyright (C) 2016 Solon Pissis and Ahmad Retha
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

#ifndef __MAXSHIFTM__
#define __MAXSHIFTM__

#include <cstdlib>
#include <cmath>
#include <cstring>
#include <stdio.h>
#include <limits.h>

using namespace std;

namespace maxshiftm
{
    /**
     * Define WORD - uses all bits available in a computer word
     */
    typedef unsigned long int WORD;

    /**
     * WORD size and inline delta function
     */
    #define BYTE_SIZE 8
    #define WORD_SIZE sizeof ( WORD ) * BYTE_SIZE
    #define delta(a,b) ( ( WORD ) ((a) != (b)) )

    /**
     * A structure to hold number of WORDs and bits needed to store m. Also contains
     * yWord mask to be used to clear the left-most bits on the most significant
     * WORD in the errors array
     */
    struct Limit
    {
	unsigned int h;
	unsigned int words;
	WORD yWord;
    };

    /**
     * The MaxShiftM class
     */
    class MaxShiftM
    {
	private:
	    void init_limit ( unsigned int factor_length );

	protected:
	    struct Limit lim;
	    unsigned char * p;
	    unsigned int m;
	    unsigned char * t;
	    unsigned int n;
	    unsigned int popcount_words ( WORD * words );
	    void shift_words ( WORD * words );
	    void shiftc_words ( WORD * words );
	    void displayMatrix ( WORD *** M );

	public:
	    MaxShiftM ( unsigned char * p, unsigned int m, unsigned char * t, unsigned int n, unsigned int h );
	    ~MaxShiftM ();
	    unsigned int maxshiftm_hd ( unsigned int * ii, unsigned int * jj, unsigned int * dd );
	    unsigned int maxshiftm_hd_ls ( unsigned int * ii, unsigned int * jj, unsigned int * dd );
    };

}

#endif
