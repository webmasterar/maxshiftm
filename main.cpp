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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include "maxshiftm.h"

using namespace maxshiftm;

int main ( int argc, char** argv )
{
    unsigned char * info = (unsigned char *)
"The valid options for MaxShiftM are:\n\
-m\tRequired. The mode (hd, hdls)\n\
-t\tRequired. The text to search through\n\
-f\tOptional. File for -t if -t won't do\n\
-p\tRequired. The pattern you are searching for\n\
-l\tOptional. File for -p if -p won't do\n\
-h\tRequired. Length of factor, e.g. 9\n";
    unsigned char * mode = (unsigned char *) "";
    unsigned char * t;
    unsigned char * p;
    unsigned int h = 0;
    FILE * ft, * fp;

    unsigned int i, a = 0;
    for ( i = 1; i < argc; i++ ) {
        if ( '-' == (int)argv[i][0] ) {
	    switch ( (int)argv[i][1] ) {
                case 'm':
                    mode = (unsigned char *) argv[i + 1];
		    a++;
                    break;

                case 't':
                    t = (unsigned char *) argv[i + 1];
		    a++;
                    break;

                case 'f':
                    if ( ( ft = fopen ( argv[i + 1], "r" ) ) == NULL ) {
                        fprintf( stderr, "Input file f could not be opened!\n");
                        return ( EXIT_FAILURE );
                    } else {
                        fseek ( ft, 0, SEEK_END );
                        unsigned long length = ftell ( ft );
                        fseek ( ft, 0, SEEK_SET );
                        t = (unsigned char *) malloc ( length );
                        if ( t ) {
                            size_t b = fread ( t, 1, length, ft );
                        }
                        fclose ( ft );
                    }
                    a++;
                    break;

                case 'p':
                    p = (unsigned char *) argv[i + 1];
		    a++;
                    break;

                case 'l':
                    if ( ( fp = fopen ( argv[i + 1], "r" ) ) == NULL ) {
                        fprintf( stderr, "Input file l could not be opened!\n");
                        return ( EXIT_FAILURE );
                    } else {
                        fseek ( fp, 0, SEEK_END );
                        unsigned long length = ftell ( fp );
                        fseek ( fp, 0, SEEK_SET );
                        p = (unsigned char *) malloc ( length );
                        if ( p ) {
                            size_t b = fread ( p, 1, length, fp );
                        }
                        fclose ( fp );
                    }
                    a++;
                    break;

                case 'h':
		    h = (unsigned int) atoi( argv[i + 1] );
		    a++;
                    break;

                default:
                    fprintf( stderr, "Invalid option '-%c' supplied!\n", argv[i][1] );
                    printf ( "%s", info );
                    return ( EXIT_FAILURE );
            }
        }
    }
    
    if ( a == 0 )
    {
	printf ( "%s", info );
	return ( EXIT_SUCCESS );
    }
    else if ( a < 4 )
    {
	fprintf( stderr, "Invalid number of options supplied!\n", argv[i][1] );
	printf ( "%s", info );
	return ( EXIT_FAILURE );
    }

    unsigned int n = strlen ( (char *) t );
    unsigned int m = strlen ( (char *) p );

    if ( strlen ( (char *) mode ) == 0 || n == 0 || m == 0 ) {
        fprintf( stderr, "Command line options missing!\n");
        printf ( "%s", info );
        return ( EXIT_FAILURE );
    }

    if ( h == 0 || ( h > m || h > n ) ) {
        fprintf( stderr, "Factor length cannot be empty or longer than the strings being searched!\n");
        return ( EXIT_FAILURE );
    }

    //stores the error-distance score and coordinates of the best match
    unsigned int ii = 0, jj = 0, ee = 0;
    
    //create MaxShiftM object ready for searching
    MaxShiftM mx ( p, m, t, n, h );
    
    if ( strcmp ( (char *) mode, "hd" ) == 0 ) {
        mx.maxshiftm_hd ( &ii, &jj, &ee );
    } else if ( strcmp ( (char *) mode, "hdls" ) == 0 ) {
        mx.maxshiftm_hd_ls ( &ii, &jj, &ee );
    } else {
        fprintf( stderr, "Invalid mode '%s' supplied!\n", mode );
        printf ( "%s", info );
        return ( EXIT_FAILURE );
    }

    printf( "e: %u, i: %u, j: %u\n", ee, ii, jj );

    return (EXIT_SUCCESS);
}
