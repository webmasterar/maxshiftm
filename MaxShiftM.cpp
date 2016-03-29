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

#include "maxshiftm.h"

using namespace std;
using namespace maxshiftm;

//
// Constructor/Destructor
//

MaxShiftM::MaxShiftM ( unsigned char * p, unsigned int m, unsigned char * t, unsigned int n, unsigned int h )
{
    this->p = p;
    this->m = m;
    this->t = t;
    this->n = n;
    this->init_limit ( h );
}

MaxShiftM::~MaxShiftM ()
{
    //free ( this->lim ); 
}

//
// Private methods
//

void MaxShiftM::init_limit ( unsigned int factor_length )
{
    unsigned int WSi = (unsigned int) WORD_SIZE;
    double WSd = (double) WORD_SIZE;
    
    this->lim.h = factor_length;
    this->lim.yWord = ULONG_MAX >> ( WSi - ( this->lim.h % WSi ) ) % WSi;    
    this->lim.words = (unsigned int) ceil ( this->lim.h / WSd );
}


//
// Protected methods
//

unsigned int MaxShiftM::popcount_words ( WORD * words )
{
    unsigned int count = 0;
    unsigned int i;
    for ( i = 0; i < this->lim.words; i++ ) {
        count += __builtin_popcountl ( words[i] );
    }
    return count;
}

void MaxShiftM::shift_words ( WORD * words )
{
    WORD mask = (WORD) 1 << ( WORD_SIZE - 1 );
    WORD carried_bit = 0;
    WORD temp;
    int i;
    for ( i = this->lim.words - 1; i > -1; i-- ) {
        temp = words[i];
        words[i] = (WORD) ( ( words[i] << 1 ) | carried_bit );
        carried_bit = (WORD) ( ( temp & mask ) != 0 );
    }
}

void MaxShiftM::shiftc_words ( WORD * words )
{
    shift_words ( words );
    words[0] = words[0] & this->lim.yWord;
}

void MaxShiftM::displayMatrix ( WORD *** M )
{
    unsigned int i, j;
    for ( i = 0; i < this->m; i++ ) {
        for ( j = 0; j < this->n; j++ ) {
            printf ( "m: %d, n: %d\t%d\n", i, j, popcount_words ( M[i][j] ) );
        }
    }
}


//
// Public methods
//

unsigned int MaxShiftM::maxshiftm_hd ( unsigned int * ii, unsigned int * jj, unsigned int * dd )
{
    unsigned int i, j, k, err, min_err = UINT_MAX;

    WORD * ones;
    if ( ( ones = ( WORD * ) calloc ( this->lim.words , sizeof ( WORD ) ) ) == NULL )
    {
        fprintf( stderr, " Error: ow could not be allocated!\n");
        return ( 0 );
    }
    
    //initialise Matrix
    WORD *** M;    
    if ( ( M = ( WORD *** ) calloc ( ( this->m + 1 ) , sizeof ( WORD ** ) ) ) == NULL )
    {
        fprintf( stderr, " Error: M1 could not be allocated!\n");
        return ( 0 );
    }
    for ( i = 0; i < this->m + 1; i++ )
    {
        if ( ( M[i] = ( WORD ** ) calloc ( ( this->n + 1 ) , sizeof ( WORD * ) ) ) == NULL )
        {
            fprintf( stderr, " Error: M2 could not be allocated!\n");
            return ( 0 );
        }
    }
    for ( i = 0; i < this->m + 1; i++ )
    {
        for ( j = 0; j < this->n + 1; j++ )
        {
            if ( ( M[i][j] = ( WORD * ) calloc ( this->lim.words , sizeof ( WORD ) ) ) == NULL )
            {
                fprintf( stderr, " Error: M3 could not be allocated!\n");
                return ( 0 );
            }
        }
    }
    
    //loop through sequences
    for ( i = 1; i < this->m + 1; i++ ) //loop through p
    {

        for ( j = 0; j < this->n + 1; j++ ) //loop through t
        {

            if ( j == 0 )
            {
                //fill up the first column with ones up to length h
                if ( i <= this->lim.h ) {
                    this->shift_words ( ones );
                    ones[this->lim.words - 1] = ones[this->lim.words - 1] + 1;
                }
                memcpy ( M[i][j], ones, this->lim.words * sizeof ( WORD ) );
            }
            else
            {
                //copy values from diagonal cell into current cell
                for ( k = 0; k < this->lim.words; k++ ) {
                    M[i][j][k] = M[i - 1][j - 1][k];
                }

                //shift things along one and clear left most bit
                this->shiftc_words ( M[i][j] );

                //set last bit on right to hamming distance of the characters
                M[i][j][this->lim.words - 1] = M[i][j][this->lim.words - 1] | delta ( this->p[i - 1], this->t[j - 1] );
            }

            if ( j >= this->lim.h && i >= this->lim.h )
            {
                err = this->popcount_words ( M[i][j] );

                if ( err < min_err )
                {
                    min_err = err;
                    ( *dd ) = min_err;
                    ( *ii ) = i - 1;
                    ( *jj ) = j - 1;
                }
            }

        }

    }

//    displayMatrix( M );

    for ( i = 0; i < this->m + 1; i++ )
    {
        for ( j = 0; j < this->n + 1; j++ ) {
            free ( M[i][j] );
        }
        free ( M[i] );
    }
    free ( M );
    free ( ones );

    return 1;
}

unsigned int MaxShiftM::maxshiftm_hd_ls (unsigned int * ii, unsigned int * jj, unsigned int * dd )
{
    unsigned int i, j, k, err, min_err = UINT_MAX;

    WORD * ones;
    if ( ( ones = ( WORD * ) calloc ( this->lim.words , sizeof ( WORD ) ) ) == NULL )
    {
        fprintf( stderr, " Error: ow could not be allocated!\n");
        return ( 0 );
    }

    //initialise 2 line matrix
    WORD ** M0;
    WORD ** M1;
    if ( ( M0 = ( WORD ** ) calloc ( ( this->n + 1 ) , sizeof ( WORD * ) ) ) == NULL )
    {
        fprintf( stderr, " Error: M0 could not be allocated!\n");
        return ( 0 );
    }
    if ( ( M1 = ( WORD ** ) calloc ( ( this->n + 1 ) , sizeof ( WORD * ) ) ) == NULL )
    {
        fprintf( stderr, " Error: M1 could not be allocated!\n");
        return ( 0 );
    }
    for ( j = 0; j < n + 1; j++ )
    {
        if ( ( M0[j] = ( WORD * ) calloc ( this->lim.words , sizeof ( WORD ) ) ) == NULL )
        {
            fprintf( stderr, " Error: M0J could not be allocated!\n");
            return ( 0 );
        }
        if ( ( M1[j] = ( WORD * ) calloc ( this->lim.words , sizeof ( WORD ) ) ) == NULL )
        {
            fprintf( stderr, " Error: M1J could not be allocated!\n");
            return ( 0 );
        }
    }

    //loop through sequences
    for ( i = 1; i < this->m + 1; i++ ) //loop through p
    {
        
        for ( j = 0; j < this->n + 1; j++ ) //loop through t
        {

            //make ones
            if ( j == 0  && i <= this->lim.h ) {
                shift_words ( ones );
                ones[this->lim.words - 1] = ones[this->lim.words - 1] + 1;
            }

            switch ( i % 2 ) {

                case 0:

                    if ( j == 0 )
                    {
                        //fill up the first column with ones up to length h
                        memcpy ( M1[j], ones, this->lim.words * sizeof ( WORD ) );
                    }
                    else
                    {
                        //copy values from diagonal cell into current cell
                        for ( k = 0; k < this->lim.words; k++ ) {
                            M1[j][k] = M0[j - 1][k];
                        }

                        //shift things along one and clear left most bit
                        shiftc_words ( M1[j] );

                        //set last bit on right to hamming distance of the characters
                        M1[j][this->lim.words - 1] = M1[j][this->lim.words - 1] | delta ( this->p[i - 1], this->t[j - 1] );
                    }

                    if ( j >= this->lim.h && i >= this->lim.h )
                    {                
                        err = popcount_words ( M1[j] );

                        if ( err < min_err )
                        {
                            min_err = err;
                            ( *dd ) = min_err;
                            ( *ii ) = i - 1;
                            ( *jj ) = j - 1;
                        }
                    }
                    break;

                case 1:

                    if ( j == 0 )
                    {
                        //fill up the first column with ones up to length h
                        memcpy ( M0[j], ones, this->lim.words * sizeof ( WORD ) );
                    }
                    else
                    {
                        //copy values from diagonal cell into current cell
                        for ( k = 0; k < this->lim.words; k++ ) {
                            M0[j][k] = M1[j - 1][k];
                        }

                        //shift things along one and clear left most bit
                        shiftc_words ( M0[j] );

                        //set last bit on right to hamming distance of the characters
                        M0[j][this->lim.words - 1] = M0[j][this->lim.words - 1] | delta ( this->p[i - 1], this->t[j - 1] );
                    }

                    if ( j >= this->lim.h && i >= this->lim.h )
                    {                
                        err = popcount_words ( M0[j] );

                        if ( err < min_err )
                        {
                            min_err = err;
                            ( *dd ) = min_err;
                            ( *ii ) = i - 1;
                            ( *jj ) = j - 1;
                        }
                    }
                    break;
            }

        }

    }

    for ( j = 0; j < this->n + 1; j++ ) {
        free ( M0[j] );
        free ( M1[j] );
    }
    free ( M0 );
    free ( M1 );
    free ( ones );

    return 1;
}
