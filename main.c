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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int main ( int argc, char** argv ) {

    unsigned char* info = "The valid options for MW-MaxShift are:\n"
                          "-m\tRequired. The mode (ed, edls, hd, hdls)\n"
                          "-t\tRequired. The text to search through\n"
                          "-f\tOptional. File for -t if -t won't do\n"
                          "-p\tRequired. The pattern you are searching for\n"
                          "-l\tOptional. File for -p if -p won't do\n"
                          "-h\tOptional. Length of factor, e.g. 9\n";
    unsigned char * mode = "";
    unsigned char * t = "";
    unsigned char * p = "";
    unsigned int h = 0;
    FILE * ft, * fp;
    
    unsigned int i;
    for ( i = 1; i < argc; i++ ) {
        if ( '-' == (int)argv[i][0] ) {
            switch ( (int)argv[i][1] ) {
                case 'm':
                    mode = argv[i + 1];
                    break;
                
                case 't':
                    t = argv[i + 1];
                    break;
                    
                case 'f':
                    if ( ( ft = fopen ( argv[i + 1], "r" ) ) == NULL ) {
                        fprintf( stderr, "Input file f could not be opened!\n");
                        return ( EXIT_FAILURE );
                    } else {
                        fseek ( ft, 0, SEEK_END );
                        unsigned long length = ftell ( ft );
                        fseek ( ft, 0, SEEK_SET );
                        t = malloc ( length );
                        if ( t ) {
                            fread ( t, 1, length, ft );
                        }
                        close ( ft );
                    }
                    break;
                    
                case 'p':
                    p = argv[i + 1];
                    break;
                    
                case 'l':
                    if ( ( fp = fopen ( argv[i + 1], "r" ) ) == NULL ) {
                        fprintf( stderr, "Input file l could not be opened!\n");
                        return ( EXIT_FAILURE );
                    } else {
                        fseek ( fp, 0, SEEK_END );
                        unsigned long length = ftell ( fp );
                        fseek ( fp, 0, SEEK_SET );
                        p = malloc ( length );
                        if ( p ) {
                            fread ( p, 1, length, fp );
                        }
                        close ( fp );
                    }
                    break;
                    
                case 'h':
                    h = (unsigned int) atoi( argv[i + 1] );
                    break;
                    
                default:
                    fprintf( stderr, "Invalid option '-%c' supplied!\n", argv[i][1] );
                    printf ( "%s", info );
                    return ( EXIT_FAILURE );
            }
        }
    }
    
    unsigned int n = strlen ( t );
    unsigned int m = strlen ( p );
    
    if ( strlen ( mode ) == 0 || n == 0 || m == 0 ) {
        fprintf( stderr, "Command line options missing!\n");
        printf ( "%s", info );
        return ( EXIT_FAILURE );
    }
    
    if ( h == 0 || ( h > m && h > n ) ) {
        fprintf( stderr, "Factor length cannot be empty or longer than the strings being searched!\n");
        return ( EXIT_FAILURE );
    }
    
    //stores the error/edit-distance score and coordinates of the best match
    unsigned int ii = 0, jj = 0, ee = 0;
    
    if ( strcmp ( mode, "hd" ) == 0 ) {
        mw_maxshift_hd ( p, m, t, n, h, &ii, &jj, &ee);
    } else if ( strcmp ( mode, "hdls" ) == 0 ) {
        mw_maxshift_hd_ls ( p, m, t, n, h, &ii, &jj, &ee);
    } else if ( strcmp( mode, "ed" ) == 0 ) {
        mw_maxshift_ed ( p, m, t, n, h, &ii, &jj, &ee );
    } else if ( strcmp ( mode, "edls" ) == 0 ) {
        mw_maxshift_ed_ls ( p, m, t, n, h, &ii, &jj, &ee );
    } else {
        fprintf( stderr, "Invalid mode '%s' supplied!\n", mode );
        printf ( "%s", info );
        return ( EXIT_FAILURE );
    }
    
    printf( "e: %u, i: %u, j: %u\n", ee, ii, jj );

    return (EXIT_SUCCESS);
}