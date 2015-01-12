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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include "mw_maxshift.h"

/**
 * Given a factor string length, this function sets the Limit structure to hold
 * the number of WORDS needed in the array as well as generating a bit mask
 * (yWord) to clear far left bits on the most significant byte
 * 
 * @param h Length of factor we are looking for
 * @param mode Edit Distance or Hamming Distance mode
 * @param lim
 * @return
 */
inline Limit init_limit ( unsigned int h, unsigned int mode, struct Limit lim )
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
    
    if ( mode == MODE_ED ) {

        if ( shift_places == 0 ) {
            lim.words++;
            lim.yIndex = 1;
        }
        
        int originalWordCount = lim.words;
        lim.words = (unsigned int) ceil ( (double) ( ( 2 * lim.h ) - 1 ) / WSd );
        lim.yIndex = lim.yIndex + ( lim.words - originalWordCount );
    }
    
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
inline unsigned int popcount_words ( WORD *words, int length )
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
inline WORD * shift_words ( WORD *words, int length )
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
inline WORD * shiftc_words ( WORD *words, struct Limit lim )
{
    words = shift_words ( words, lim.words );
    words[lim.yIndex] = words[lim.yIndex] & lim.yWord;
    return words;
}

/**
 * Edit Distance model function that shifts bits left one position then
 * truncates left-most bits on the yIndex WORD of the error array using the
 * Limit.yWord mask. The extended yIndex and yWord are calculated using the
 * initialized limit and the (h) factor in xWord by calling extend_edx_limit()
 * 
 * @param e
 * @param lim
 * @return
 */
inline xWord * shiftc_edx_words ( xWord *e, Limit lim )
{
    Limit myLim = lim;
    unsigned int h = ( * e ).h;
    myLim = extend_edx_limit ( myLim, (unsigned int)( h - lim.h ) );
    ( * e ).errors = shift_words ( ( * e ).errors, myLim.words );
    ( * e ).errors[myLim.yIndex] = ( * e ).errors[myLim.yIndex] & myLim.yWord;
    return e;
}

/**
 * Finds out which of the word arrays has the highest numerical value /
 * left-most set bit
 *
 * @param a
 * @param b
 * @return
 */
inline WORD * bitminmax_numerical_words ( WORD *a, WORD *b, unsigned int length )
{
    unsigned int i;
    for ( i = 0; i < length; i++ ) {
        if ( a[i] > b[i] ) {
            return a;
        } else if ( b[i] > a[i] ) {
            return b;
        }
    }
    
    //both the same so just return a
    return a;
}

/**
 * Carries out a bitminmax operation on xWord objects
 * 
 * @param a
 * @param b
 * @param c
 * @param length Number of elements in the arrays (expect equal length)
 * @return 
 */
inline xWord * bitminmax_edx_words ( xWord *a, xWord *b, xWord *c, unsigned int length )
{
    WORD * winner = bitminmax_words ( ( * a ).errors, ( * b ).errors, ( * c ).errors, length );
    if ( winner == ( * a ).errors ) {
        return a;
    } else if ( winner == ( * b ).errors ) {
        return b;
    } else {
        return c;
    }
}

/**
 * Returns one of the WORD arrays a, b, or c if it has the least number of 1's
 * (popcount). If there is a draw between two or three of them, it takes the
 * smallest popcount pair and compares them as decimal integers and returns the
 * one the maximum value in the most significant word
 * 
 * @param a
 * @param b
 * @param c
 * @param length Number of elements in the arrays (expect equal length)
 * @return 
 */
inline WORD * bitminmax_words ( WORD *a, WORD *b, WORD *c, unsigned int length )
{
    WORD x = 0, y = 0, z = 0;

    //this is an optimisation instead of running popcount_words() for each element individually
    unsigned int i;
    for ( i = 0; i < length; i++ ) {
        x += __builtin_popcountl ( a[i] );
        y += __builtin_popcountl ( b[i] );
        z += __builtin_popcountl ( c[i] );
    }
    
    //The one with the lowest popcount is returned. If 3 or 2 of them draw, then
    //the those two with the lowest popcount are passed to bitminmax_numerical_words()
    //which returns the one from the pair with the left-most set bit
    if ( x == y && y == z ) {
        WORD * d = bitminmax_numerical_words ( a, b, length );
        return bitminmax_numerical_words ( d, c, length );
    } else if ( x == y && x < z ) {
        return bitminmax_numerical_words ( a, b, length );
    } else if ( y == z && y < x ) {
        return bitminmax_numerical_words ( b, c, length );
    } else if ( x == z && x < y ) {
        return bitminmax_numerical_words ( a, c, length );
    } else if ( x < y && x < z ) {
        return a;
    } else if ( y < x && y < z ) {
        return b;
    } else { // if ( z < x && z < y )
        return c;
    }
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
inline void displayMatrix ( WORD ***M, unsigned int m, unsigned int n, unsigned int o )
{
    unsigned int i, j;
    for ( i = 0; i < m; i++ ) {
        for ( j = 0; j < n; j++ ) {
            printf ( "m: %d, n: %d\t%d\n", i, j, popcount_words ( M[i][j], o ) );
        }
    }
}

/**
 * Debugging function: prints out the elements in the Dynamic programming matrix
 * for the Edit Distance model
 * 
 * @param M Dynamic Programming matrix
 * @param m Number of elements in array
 * @param n Number of elements in array[a]
 * @param o Number of elements in array[a][b]
 */
inline void display_edx_matrix ( xWord ***M, unsigned int m, unsigned int n, unsigned int o )
{
    unsigned int i, j;
    for ( i = 0; i < m; i++ ) {
        for ( j = 0; j < n; j++ ) {
            printf ( "m: %d, n: %d\t%d\n", i, j, popcount_words ( ( * M[i][j] ).errors, o ) );
        }
    }
}

/**
 * In the Edit Distance model, when the left cell is the winner of bitminmax we
 * extend the factor length by 1 and update the yIndex and yWord mask accordingly
 *
 * @param lim
 * @param times How much longer than the original factor length it should be
 * @return
 */
inline Limit extend_edx_limit ( Limit lim, unsigned int times )
{
    unsigned int i = 0;
    while ( i < times ) {
        //extend the limit
        lim.h++;

        //when factor length is word size, move to previous index and set mask
        //to 1, otherwise just shift yWord mask along one
        if ( lim.yWord == ULONG_MAX && lim.yIndex > 0 ) {
             lim.yIndex--;
             lim.yWord = (WORD)1;
        } else {
             lim.yWord = ( lim.yWord << 1 ) + (WORD)1;
        }
        i++;
    }
    
    return lim;
}

/**
 * MW-MaxShift under the the Edit Distance model
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
inline unsigned int mw_maxshift_ed ( unsigned char *p, unsigned int m, unsigned char *t, unsigned int n, unsigned int h, unsigned int *ii, unsigned int *jj, unsigned int *dd )
{
    //initialise variables
    unsigned int i, j, k, err, min_err = UINT_MAX;
    Limit lim;
    lim = init_limit ( h, MODE_ED, lim );
    WORD * ones;
    if ( ( ones = ( WORD * ) calloc ( lim.words , sizeof( WORD ) ) ) == NULL )
    {
        fprintf( stderr, " Error: ow could not be allocated!\n");
        return ( 0 );
    }

    //initialise Matrix
    xWord *** M;
    if ( ( M = ( xWord *** ) calloc ( ( m + 1 ) , sizeof ( xWord ** ) ) ) == NULL )
    {
        fprintf ( stderr, " Error: M1 could not be allocated!\n");
        return ( 0 );
    }
    for ( i = 0; i < m + 1; i ++ ) 
    {
        if ( ( M[i] = ( xWord ** ) calloc ( ( n + 1 ) , sizeof ( xWord * ) ) ) == NULL )
        {
            fprintf ( stderr, " Error: M2 could not be allocated!\n");
            return ( 0 );
        }
    }
    for ( i = 0; i < m + 1; i ++ ) 
    {
        for ( j = 0; j < n + 1; j ++ )
        {
            if ( ( M[i][j] = ( xWord * ) malloc ( sizeof ( xWord ) ) ) == NULL )
            {
                fprintf ( stderr, " Error: M3 could not be allocated!\n");
                return ( 0 );
            }
        }
    }
    
    //create variables to hold temporary values for left, diagonal and top cells of dynamic programming matrix
    xWord * lc;
    xWord * dc;
    xWord * tc;
    xWord * oc;
    if ( ( lc = ( xWord * ) malloc ( sizeof ( xWord ) ) ) == NULL )
    {
        fprintf ( stderr, " Error: lc could not be allocated!\n");
        return ( 0 );
    }
    if ( ( dc = ( xWord * ) malloc ( sizeof ( xWord ) ) ) == NULL )
    {
        fprintf ( stderr, " Error: dc could not be allocated!\n");
        return ( 0 );
    }
    if ( ( tc = ( xWord * ) malloc ( sizeof ( xWord ) ) ) == NULL )
    {
        fprintf ( stderr, " Error: tc could not be allocated!\n");
        return ( 0 );
    }
    //initialise errors array
    if ( ( ( * lc ).errors = ( WORD * ) calloc ( lim.words , sizeof ( WORD ) ) ) == NULL )
    {
        fprintf ( stderr, " Error: lc.errors could not be allocated!\n");
        return ( 0 );
    }
    if ( ( ( * dc ).errors = ( WORD * ) calloc ( lim.words , sizeof ( WORD ) ) ) == NULL )
    {
        fprintf ( stderr, " Error: dc.errors could not be allocated!\n");
        return ( 0 );
    }
    if ( ( ( * tc ).errors = ( WORD * ) calloc ( lim.words , sizeof ( WORD ) ) ) == NULL )
    {
        fprintf ( stderr, " Error: tc.errors could not be allocated!\n");
        return ( 0 );
    }

    //loop through sequences
    for ( i = 0; i < m + 1; i++ ) //loop through m
    {
        
        for ( j = 0; j < n + 1; j++ ) //loop through t
        {
            
            //initialise error matrix and factor length
            if ( ( ( * M[i][j] ).errors = ( WORD * ) calloc ( lim.words, sizeof ( WORD ) ) ) == NULL )
            {
                fprintf ( stderr, " Error: eMij.errors could not be allocated!\n");
                return ( 0 );
            }
            ( * M[i][j] ).h = lim.h;

            if ( i == 0 )
            {
                 continue;
            }
            else if ( j == 0 )
            {
                //fill up the first column with ones either to the size of i or h
                if ( i <= lim.h ) {
                    ones = shift_words ( ones, lim.words );
                    ones[lim.words - 1] = ones[lim.words - 1] + (WORD)1;
                }
                for ( k = 0; k < lim.words; k++ ) {
                    ( * M[i][j] ).errors[k] = ones[k];
                }
                continue;
            }
            else
            {
                //identify which cell has the min popcount - left, diagonal or top cell (dynamic programming matrix)
                for ( k = 0; k < lim.words; k++ ) {
                    ( * lc ).errors[k] = ( * M[i][j - 1] ).errors[k];
                    ( * dc ).errors[k] = ( * M[i - 1][j - 1] ).errors[k];
                    ( * tc ).errors[k] = ( * M[i - 1][j] ).errors[k];
                }
                ( * lc ).h = ( * M[i][j - 1] ).h;
                ( * dc ).h = ( * M[i - 1][j - 1] ).h;
                ( * tc ).h = ( * M[i - 1][j] ).h;

                if ( i <= lim.h ) {
                    ( * dc ).errors = shift_words ( ( * dc ).errors, lim.words );
                    ( * tc ).errors = shift_words ( ( * tc ).errors, lim.words );
                } else {
                    dc = shiftc_edx_words ( dc, lim );
                    tc = shiftc_edx_words ( tc, lim );
                }
                ( * lc ).errors = shift_words ( ( * lc ).errors, lim.words );
                ( * lc ).errors[lim.words - 1] = ( * lc ).errors[lim.words - 1] | (WORD)1;
                ( * dc ).errors[lim.words - 1] = ( * dc ).errors[lim.words - 1] | (WORD)delta ( p[i - 1], t[j - 1] );
                ( * tc ).errors[lim.words - 1] = ( * tc ).errors[lim.words - 1] | (WORD)1;

                oc = bitminmax_edx_words ( dc, tc, lc, lim.words );

                //if left cell wins extend its error factor length
                if ( oc == lc && ( * lc ).h < ( (2 * lim.h ) - 1 ) ) {
                    ( * lc ).h++;
                }

                for ( k = 0; k < lim.words; k++ ) {
                    ( * M[i][j] ).errors[k] = ( * oc ).errors[k];
                }
                ( * M[i][j] ).h = ( * oc ).h;
                
            }
            
            if ( i >= lim.h )
            {
                err = popcount_words ( ( * M[i][j] ).errors, lim.words );

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

//    display_edx_matrix ( M, m + 1, n + 1, lim.words );

    for ( i = 0; i < m + 1; i ++ ) 
    {
        for ( j = 0; j < n + 1; j ++ ) {
            free ( ( * M[i][j] ).errors );
            free ( M[i][j] );
        }
        free ( M[i] );
    }
    free ( M );

    free ( ( * lc ).errors );
    free ( lc );
    free ( ( * tc ).errors );
    free ( tc );
    free ( ( * dc ).errors );
    free ( dc );
    free ( ones );

    return 1;
}

/**
 * MW-MaxShift under the the Hamming Distance model
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
inline unsigned int mw_maxshift_hd ( unsigned char * p, unsigned int m, unsigned char * t, unsigned int n, unsigned int h, unsigned int *ii, unsigned int *jj, unsigned int *dd )
{
    unsigned int i, j, k, err, min_err;
    min_err = UINT_MAX;
    Limit lim;
    lim = init_limit ( h, MODE_HD, lim );
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
inline unsigned int mw_maxshift_hd_ls ( unsigned char * p, unsigned int m, unsigned char * t, unsigned int n, unsigned int h, unsigned int *ii, unsigned int *jj, unsigned int *dd )
{
    unsigned int i, j, k, err, min_err;
    min_err = UINT_MAX;
    Limit lim;
    lim = init_limit ( h, MODE_HD, lim );
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

/**
 * MW-MaxShift under the the Edit Distance model. Linear Space memory usage
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
inline unsigned int mw_maxshift_ed_ls ( unsigned char * p, unsigned int m, unsigned char * t, unsigned int n, unsigned int h, unsigned int *ii, unsigned int *jj, unsigned int *dd )
{
    //initialise variables
    unsigned int i, j, k, err, min_err = UINT_MAX;
    Limit lim;
    lim = init_limit ( h, MODE_ED, lim );
    WORD * ones;
    if ( ( ones = ( WORD * ) calloc ( lim.words , sizeof( WORD ) ) ) == NULL )
    {
        fprintf( stderr, " Error: ow could not be allocated!\n");
        return ( 0 );
    }
    
    //initialise 2 line matrix
    xWord ** M0;
    xWord ** M1;
    if ( ( M0 = ( xWord ** ) calloc ( ( n + 1 ) , sizeof ( xWord * ) ) ) == NULL )
    {
        fprintf( stderr, " Error: M0 could not be allocated!\n");
        return ( 0 );
    }
    if ( ( M1 = ( xWord ** ) calloc ( ( n + 1 ) , sizeof ( xWord * ) ) ) == NULL )
    {
        fprintf( stderr, " Error: M1 could not be allocated!\n");
        return ( 0 );
    }
    for ( j = 0; j < n + 1; j ++ )
    {
        if ( ( M0[j] = ( xWord * ) calloc ( lim.words , sizeof ( xWord ) ) ) == NULL )
        {
            fprintf( stderr, " Error: M0J could not be allocated!\n");
            return ( 0 );
        }
        if ( ( ( * M0[j] ).errors = ( WORD * ) calloc ( lim.words, sizeof ( WORD ) ) ) == NULL )
        {
            fprintf ( stderr, " Error: M0Je.errors could not be allocated!\n");
            return ( 0 );
        }
        ( * M0[j] ).h = lim.h;
        if ( ( M1[j] = ( xWord * ) calloc ( lim.words , sizeof ( xWord ) ) ) == NULL )
        {
            fprintf( stderr, " Error: M1J could not be allocated!\n");
            return ( 0 );
        }
        if ( ( ( * M1[j] ).errors = ( WORD * ) calloc ( lim.words, sizeof ( WORD ) ) ) == NULL )
        {
            fprintf ( stderr, " Error: M1Je.errors could not be allocated!\n");
            return ( 0 );
        }
        ( * M1[j] ).h = lim.h;
    }

    //create variables to hold temporary values for left, diagonal and top cells of dynamic programming matrix
    xWord * lc;
    xWord * dc;
    xWord * tc;
    xWord * oc;
    if ( ( lc = ( xWord * ) malloc ( sizeof ( xWord ) ) ) == NULL )
    {
        fprintf ( stderr, " Error: lc could not be allocated!\n");
        return ( 0 );
    }
    if ( ( dc = ( xWord * ) malloc ( sizeof ( xWord ) ) ) == NULL )
    {
        fprintf ( stderr, " Error: dc could not be allocated!\n");
        return ( 0 );
    }
    if ( ( tc = ( xWord * ) malloc ( sizeof ( xWord ) ) ) == NULL )
    {
        fprintf ( stderr, " Error: tc could not be allocated!\n");
        return ( 0 );
    }
    //initialise errors array
    if ( ( ( * lc ).errors = ( WORD * ) calloc ( lim.words , sizeof ( WORD ) ) ) == NULL )
    {
        fprintf ( stderr, " Error: lc.errors could not be allocated!\n");
        return ( 0 );
    }
    if ( ( ( * dc ).errors = ( WORD * ) calloc ( lim.words , sizeof ( WORD ) ) ) == NULL )
    {
        fprintf ( stderr, " Error: dc.errors could not be allocated!\n");
        return ( 0 );
    }
    if ( ( ( * tc ).errors = ( WORD * ) calloc ( lim.words , sizeof ( WORD ) ) ) == NULL )
    {
        fprintf ( stderr, " Error: tc.errors could not be allocated!\n");
        return ( 0 );
    }

    //loop through sequences
    for ( i = 1; i < m + 1; i++ ) //loop through m
    {
        
        for ( j = 0; j < n + 1; j++ ) //loop through t
        {
            //make ones
            if ( j == 0  && i <= lim.h ) {
                ones = shift_words ( ones, lim.words );
                ones[lim.words - 1] = ones[lim.words - 1] + (WORD)1;
            }
            
            switch ( i % 2 ) {
                
                case 0:
            
                    if ( j == 0 )
                    {
                        //fill up the first column with ones up to length h
                        memcpy ( ( * M1[j] ).errors, ones, lim.words * sizeof ( WORD ) );
                        continue;
                    }
                    else
                    {
                        //identify which cell has the min popcount - left, diagonal or top cell (dynamic programming matrix)
                        for ( k = 0; k < lim.words; k++ ) {
                            ( * lc ).errors[k] = ( * M1[j - 1] ).errors[k];
                            ( * dc ).errors[k] = ( * M0[j - 1] ).errors[k];
                            ( * tc ).errors[k] = ( * M0[j] ).errors[k];
                        }
                        ( * lc ).h = ( * M1[j - 1] ).h;
                        ( * dc ).h = ( * M0[j - 1] ).h;
                        ( * tc ).h = ( * M0[j] ).h;
                        
                        if ( i <= lim.h ) {
                            ( * dc ).errors = shift_words ( ( * dc ).errors, lim.words );
                            ( * tc ).errors = shift_words ( ( * tc ).errors, lim.words );
                        } else {
                            dc = shiftc_edx_words ( dc, lim );
                            tc = shiftc_edx_words ( tc, lim );
                        }
                        ( * lc ).errors = shift_words ( ( * lc ).errors, lim.words );
                        ( * lc ).errors[lim.words - 1] = ( * lc ).errors[lim.words - 1] | (WORD)1;
                        ( * dc ).errors[lim.words - 1] = ( * dc ).errors[lim.words - 1] | (WORD)delta ( p[i - 1], t[j - 1] );
                        ( * tc ).errors[lim.words - 1] = ( * tc ).errors[lim.words - 1] | (WORD)1;
        
                        oc = bitminmax_edx_words ( dc, tc, lc, lim.words );

                        //if left cell wins then extend its error factor length
                        if ( oc == lc && ( * lc ).h < ( ( 2 * lim.h ) - 1 ) ) {
                            ( * lc ).h++;
                        }
                        
                        for ( k = 0; k < lim.words; k++ ) {
                            ( * M1[j] ).errors[k] = ( * oc ).errors[k];
                        }
                        ( * M1[j] ).h = ( * oc ).h;
                    }

                    if ( i >= lim.h )
                    {                
                        err = popcount_words ( ( * M1[j] ).errors, lim.words );

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
            
                case 1:
                    
                    if ( j == 0 )
                    {
                        //fill up the first column with ones up to length h
                        memcpy ( ( * M0[j] ).errors, ones, lim.words * sizeof ( WORD ) );
                        continue;
                    }
                    else
                    {
                        //identify which cell has the max value - left, diagonal or top cell (dynamic programming matrix)
                        for ( k = 0; k < lim.words; k++ ) {
                            ( * lc ).errors[k] = ( * M0[j - 1] ).errors[k];
                            ( * dc ).errors[k] = ( * M1[j - 1] ).errors[k];
                            ( * tc ).errors[k] = ( * M1[j] ).errors[k];
                        }
                        ( * lc ).h = ( * M0[j - 1] ).h;
                        ( * dc ).h = ( * M1[j - 1] ).h;
                        ( * tc ).h = ( * M1[j] ).h;

                        if ( i <= lim.h ) {
                            ( * dc ).errors = shift_words ( ( * dc ).errors, lim.words );
                            ( * tc ).errors = shift_words ( ( * tc ).errors, lim.words );
                        } else {
                                dc = shiftc_edx_words ( dc, lim );
                                tc = shiftc_edx_words ( tc, lim );
                        }
                        ( * lc ).errors = shift_words ( ( * lc ).errors, lim.words );
                        ( * lc ).errors[lim.words - 1] = ( * lc ).errors[lim.words - 1] | (WORD)1;
                        ( * dc ).errors[lim.words - 1] = ( * dc ).errors[lim.words - 1] | (WORD)delta ( p[i - 1], t[j - 1] );
                        ( * tc ).errors[lim.words - 1] = ( * tc ).errors[lim.words - 1] | (WORD)1;
        
                        oc = bitminmax_edx_words ( dc, tc, lc, lim.words );

                        //if left cell wins then extended the error limit
                        if ( oc == lc && ( * lc ).h < ( ( 2 * lim.h ) - 1 ) ) {
                            ( * lc ).h++;
                        }
                        
                        for ( k = 0; k < lim.words; k++ ) {
                            ( * M0[j] ).errors[k] = ( * oc ).errors[k];
                        }
                        ( * M0[j] ).h = ( * oc ).h;
                    }

                    if ( i >= lim.h )
                    {                
                        err = popcount_words ( ( * M0[j] ).errors, lim.words );

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
        free ( ( * M0[j] ).errors );
        free ( M0[j] );
        free ( ( * M1[j] ).errors );
        free ( M1[j] );
    }
    free ( M0 );
    free ( M1 );
    
    free ( ( * lc ).errors );
    free ( lc );
    free ( ( * tc ).errors );
    free ( tc );
    free ( ( * dc ).errors );
    free ( dc );
    free ( ones );
    
    return 1;
}
