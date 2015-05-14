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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include "maxshiftm.h"

/**
 * Given a factor string length, this function sets the Limit structure to hold
 * the number of WORDS needed in the array as well as generating a bit mask
 * (yWord) to clear far left bits on the most significant byte
 * 
 * @param h Length of factor we are looking for
 * @param lim
 * @return
 */
inline Limit init_limit ( unsigned int h, struct Limit lim )
{
    double WSd = (double) WORD_SIZE;
    unsigned int WSi = (unsigned int) WORD_SIZE;
    
    lim.h = h;
    
    WORD yWord = ULONG_MAX; //111111
    unsigned int shift_places = ( WSi - ( h % WSi ) ) % WSi;
    yWord = yWord >> shift_places; //if shift_places is 2: 001111
    lim.yWord = yWord;
    
    lim.words = (unsigned int) ceil ( (double) lim.h / WSd );
    lim.yIndex = 0;
    
    return lim;
}

/**
 * If you supply a WORD array it returns the sum of the popcount on them
 *  
 * See <a href="http://www.dalkescientific.com/writings/diary/archive/2011/11/02/faster_popcount_update.html">Faster popcount</a>
 * 
 * @param words WORD Array
 * @param length Number of elements in the WORD array
 * @return Sum of popcounts on a WORD array
 */
inline unsigned int popcount_words ( WORD * words, int length )
{
    unsigned int count = 0;
    unsigned int i;
    for ( i = 0; i < length; i++ ) {
        count += __builtin_popcountl ( words[i] );
    }
    return count;
}

/**
 * Shifts bits in an array of WORDs one position to the left
 * 
 * @param words
 * @param length Number of elements in the WORD array
 * @return 
 */
inline WORD * shift_words ( WORD * words, int length )
{
    WORD mask = (WORD) 1 << ( (int) WORD_SIZE - 1 );
    WORD carried_bit = 0;
    WORD temp;
    int i;
    for ( i = length - 1; i > -1; i-- ) {
        temp = words[i];
        words[i] = (WORD) ( ( words[i] << 1 ) | carried_bit );
        carried_bit = (WORD) ( ( temp & mask ) != 0 );
    }
    return words;
}

/**
 * Shifts bits left one position then truncates left-most bits on most
 * significant WORD of array using Limit.yWord mask
 * 
 * @param words WORDs array
 * @param lim An initialised Limit structure
 * @return 
 */
inline WORD * shiftc_words ( WORD * words, struct Limit lim )
{
    words = shift_words ( words, lim.words );
    words[lim.yIndex] = words[lim.yIndex] & lim.yWord;
    return words;
}

/**
 * Returns the hamming distance (1 or 0) for two characters a and b
 * 
 * @param a
 * @param b
 * @return Hamming distance between characters given
 */
inline WORD delta ( char a, char b )
{
    return (WORD)( a != b );
}

/**
 * Debugging function: prints out the elements in the Dynamic programming matrix
 * 
 * @param M Dynamic Programming matrix
 * @param m Number of elements in array
 * @param n Number of elements in array[a]
 * @param o Number of elements in array[a][b]
 */
inline void displayMatrix ( WORD *** M, unsigned int m, unsigned int n, unsigned int o )
{
    unsigned int i, j;
    for ( i = 0; i < m; i++ ) {
        for ( j = 0; j < n; j++ ) {
            printf ( "m: %d, n: %d\t%d\n", i, j, popcount_words ( M[i][j], o ) );
        }
    }
}

/**
 * MaxShiftM under the the Hamming Distance model
 * 
 * @param p The search pattern string
 * @param m The length of p
 * @param t The long text string
 * @param n The length of t
 * @param h The length of the factor we are looking for
 * @param ii End position of discovered match in p
 * @param jj End position of discovered match in t
 * @param dd Hamming distance
 * @return 
 */
inline unsigned int maxshiftm_hd ( unsigned char * p, unsigned int m, unsigned char * t, unsigned int n, unsigned int h, unsigned int * ii, unsigned int * jj, unsigned int * dd )
{
    unsigned int i, j, k, err, min_err;
    min_err = UINT_MAX;
    Limit lim;
    lim = init_limit ( h, lim );
    WORD * ones;
    if ( ( ones = ( WORD * ) calloc ( lim.words , sizeof ( WORD ) ) ) == NULL )
    {
        fprintf( stderr, " Error: ow could not be allocated!\n");
        return ( 0 );
    }
    
    //initialise Matrix
    WORD *** M;    
    if ( ( M = ( WORD *** ) calloc ( ( m + 1 ) , sizeof ( WORD ** ) ) ) == NULL )
    {
        fprintf( stderr, " Error: M1 could not be allocated!\n");
        return ( 0 );
    }
    for ( i = 0; i < m + 1; i ++ )
    {
        if ( ( M[i] = ( WORD ** ) calloc ( ( n + 1 ) , sizeof ( WORD * ) ) ) == NULL )
        {
            fprintf( stderr, " Error: M2 could not be allocated!\n");
            return ( 0 );
        }
    }
    for ( i = 0; i < m + 1; i ++ )
    {
        for ( j = 0; j < n + 1; j ++ )
        {
            if ( ( M[i][j] = ( WORD * ) calloc ( lim.words , sizeof ( WORD ) ) ) == NULL )
            {
                fprintf( stderr, " Error: M3 could not be allocated!\n");
                return ( 0 );
            }
        }
    }
    
    //loop through sequences
    for ( i = 1; i < m + 1; i++ ) //loop through m
    {
        
        for ( j = 0; j < n + 1; j++ ) //loop through t
        {
            if ( j == 0 )
            {
                //fill up the first column with ones up to length h
                if ( i <= lim.h ) {
                    ones = shift_words ( ones, lim.words );
                    ones[lim.words - 1] = ones[lim.words - 1] + 1;
                }
                memcpy ( M[i][j], ones, lim.words * sizeof ( WORD ) );
            }
            else
            {
                //copy values from diagonal cell into current cell
                for ( k = 0; k < lim . words; k++ ) {
                    M[i][j][k] = M[i - 1][j - 1][k];
                }

                //shift things along one and clear left most bit
                M[i][j] = shiftc_words ( M[i][j], lim );
                
                //set last bit on right to hamming distance of the characters
                M[i][j][lim.words - 1] = M[i][j][lim.words - 1] | delta ( p[i - 1], t[j - 1] );
            }
            
            if ( j >= lim.h && i >= lim.h )
            {                
                err = popcount_words ( M[i][j], lim.words );

                if ( err < min_err )
                {
                    min_err = err;
                    ( *dd ) = min_err;
                    ( *ii ) = i - 1;
                    ( *jj ) = j - 1;
                    
                    //exact match discovered. Exit search
//                    if ( min_err == 0 ) {
//                        return 1;
//                    }
                }
            }
        }
    }
    
//    displayMatrix( M, m + 1, n + 1, lim.words );
    
    for ( i = 0; i < m + 1; i ++ ) 
    {
        for ( j = 0; j < n + 1; j ++ ) {
            free ( M[i][j] );
        }
        free ( M[i] );
    }
    free ( M );
    free ( ones );
    
    return 1;
}


/**
 * MW-MaxShift under the the Hamming Distance model. Linear Space memory usage
 * 
 * @param p The search pattern string
 * @param m The length of p
 * @param t The long text string
 * @param n The length of t
 * @param h The length of the factor we are looking for
 * @param ii End position of discovered match in p
 * @param jj End position of discovered match in t
 * @param dd Edit/Hamming distance
 * @return 
 */
inline unsigned int maxshiftm_hd_ls ( unsigned char * p, unsigned int m, unsigned char * t, unsigned int n, unsigned int h, unsigned int * ii, unsigned int * jj, unsigned int * dd )
{
    unsigned int i, j, k, err, min_err;
    min_err = UINT_MAX;
    Limit lim;
    lim = init_limit ( h, lim );
    WORD * ones;
    if ( ( ones = ( WORD * ) calloc ( lim.words , sizeof ( WORD ) ) ) == NULL )
    {
        fprintf( stderr, " Error: ow could not be allocated!\n");
        return ( 0 );
    }
    
    //initialise 2 line matrix
    WORD ** M0;
    WORD ** M1;
    if ( ( M0 = ( WORD ** ) calloc ( ( n + 1 ) , sizeof ( WORD * ) ) ) == NULL )
    {
        fprintf( stderr, " Error: M0 could not be allocated!\n");
        return ( 0 );
    }
    if ( ( M1 = ( WORD ** ) calloc ( ( n + 1 ) , sizeof ( WORD * ) ) ) == NULL )
    {
        fprintf( stderr, " Error: M1 could not be allocated!\n");
        return ( 0 );
    }
    for ( j = 0; j < n + 1; j ++ )
    {
        if ( ( M0[j] = ( WORD * ) calloc ( lim.words , sizeof ( WORD ) ) ) == NULL )
        {
            fprintf( stderr, " Error: M0J could not be allocated!\n");
            return ( 0 );
        }
        if ( ( M1[j] = ( WORD * ) calloc ( lim.words , sizeof ( WORD ) ) ) == NULL )
        {
            fprintf( stderr, " Error: M1J could not be allocated!\n");
            return ( 0 );
        }
    }
    
    //loop through sequences
    for ( i = 1; i < m + 1; i++ ) //loop through m
    {
        
        for ( j = 0; j < n + 1; j++ ) //loop through t
        {
            //make ones
            if ( j == 0  && i <= lim.h ) {
                ones = shift_words ( ones, lim.words );
                ones[lim.words - 1] = ones[lim.words - 1] + 1;
            }
            
            switch ( i % 2 ) {
            
                case 0:
                
                    if ( j == 0 )
                    {
                        //fill up the first column with ones up to length h
                        memcpy ( M1[j], ones, lim.words * sizeof ( WORD ) );
                    }
                    else
                    {
                        //copy values from diagonal cell into current cell
                        for ( k = 0; k < lim.words; k++ ) {
                            M1[j][k] = M0[j - 1][k];
                        }

                        //shift things along one and clear left most bit
                        M1[j] = shiftc_words ( M1[j], lim );

                        //set last bit on right to hamming distance of the characters
                        M1[j][lim.words - 1] = M1[j][lim.words - 1] | delta ( p[i - 1], t[j - 1] );
                    }

                    if ( j >= lim.h && i >= lim.h )
                    {                
                        err = popcount_words ( M1[j], lim.words );

                        if ( err < min_err )
                        {
                            min_err = err;
                            ( *dd ) = min_err;
                            ( *ii ) = i - 1;
                            ( *jj ) = j - 1;
                        }
                        
                        //exact match discovered. Exit search
//                        if ( min_err == 0 ) {
//                            return 1;
//                        }
                    }
                    break;
                    
                case 1:
                    
                    if ( j == 0 )
                    {
                        //fill up the first column with ones up to length h
                        memcpy ( M0[j], ones, lim.words * sizeof ( WORD ) );
                    }
                    else
                    {
                        //copy values from diagonal cell into current cell
                        for ( k = 0; k < lim.words; k++ ) {
                            M0[j][k] = M1[j - 1][k];
                        }

                        //shift things along one and clear left most bit
                        M0[j] = shiftc_words ( M0[j], lim );

                        //set last bit on right to hamming distance of the characters
                        M0[j][lim.words - 1] = M0[j][lim.words - 1] | delta ( p[i - 1], t[j - 1] );
                    }

                    if ( j >= lim.h && i >= lim.h )
                    {                
                        err = popcount_words ( M0[j], lim.words );

                        if ( err < min_err )
                        {
                            min_err = err;
                            ( *dd ) = min_err;
                            ( *ii ) = i - 1;
                            ( *jj ) = j - 1;
                            
                            //exact match discovered. Exit search
//                            if ( min_err == 0 ) {
//                                return 1;
//                            }
                        }
                    }
                    break;
            }
            
        }
    }
    
    for ( j = 0; j < n + 1; j ++ ) {
        free ( M0[j] );
        free ( M1[j] );
    }
    free ( M0 );
    free ( M1 );
    free ( ones );
    
    return 1;
}

