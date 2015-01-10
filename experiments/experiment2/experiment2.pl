#!/usr/bin/perl

#The MIT License (MIT)
#
#Copyright (c) 2015 Solon Pissis and Ahmad Retha
#
#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in
#all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#THE SOFTWARE.

use strict;
use warnings;
use Time::HiRes qw(time);
use Text::Levenshtein qw(distance);

#MW-MaxShift
my $exePath = "../../dist/";
my $exe = "mw_maxshift";

#modes - edit distance and hamming distance
my @modes = ("ed", "hd");

#percent dissimilarity between read and target sequence
my @diss = (10, 20, 30, 40);

#factor lengths
my @hlens = (16, 32, 64, 128, 256, 512); #1000

#sequences to be matched
my $tSeq = "";
my $pSeq = "";

for my $mode (@modes) {

    for my $pc (@diss) {

        #open file and read lines of file into t and p seq
        my $inFile = "./data_${mode}_${pc}pc.fasta";
        open (IF, "<", $inFile) or die("Could not open file: $inFile");

        my $line = 1;
        while (<IF>) {
            chomp;
            if ($line == 2) {
                $tSeq = $_;
            } elsif ($line == 4) {
                $pSeq = $_;
            }
            $line++;
        }

        close(IF);

        #Run tSeq and pSeq through Levenshtein to get original Edit Distance score
        my $originalEditDistance = distance($tSeq, $pSeq);
        
        #randomly rotate and duplicate $pSeq
        my $origTSeq = $tSeq;
        my $tSeqLen = length($origTSeq);
        my $origPSeq = $pSeq;
        my $pSeqLen = length($origPSeq);
        my $min = 10;
        my $max = $pSeqLen - $min;
        my $randRotPos = $min + int(rand($max - $min));
        $pSeq = substr($pSeq, $randRotPos) . substr($pSeq, 0, $randRotPos);
        $pSeq = $pSeq . $pSeq;

        #holds a hash with factor lengths for keys and the edit distance scores as values
        my %editDistanceScores = ();
        
        #open file to save output to
        my $outFile = "./output_${mode}_${pc}.txt";
        open(OF, ">", $outFile) or die("Could not open file: $outFile");
        
        for my $h (@hlens) {
            #run command
            my $cmd = "$exePath$exe -m ${mode}ls -t $tSeq -p $pSeq -h $h";
            my $start = time();
            my $cmdOutput = `$cmd`;
            chop($cmdOutput);
            my $finish = (time() - $start) * 1000;

            #read string from output into vars
            $cmdOutput =~ m/e: (\d+), i: (\d+), j: (\d+)/;
            my $dd = $1;
            my $ii = $2;
            my $jj = $3;

            #rotate $pSeq sequence back to what mw_maxshift says it should be into $rSeq
            my $rSeq;
            if ($dd == $ii && $ii == $jj && $jj == 0) {
                $rSeq = "N/A";
            } else {
                #calculate rotation position
                my $pos = 0;
                if ($ii >= $jj) {
                    $pos = $ii - $jj;
                } else {
                    my $a = $jj - $ii;
                    my $b = $pSeqLen - $ii - 1;
                    my $c = ( $a <= $b ) ? $a : $b;
                    $pos = $pSeqLen - $c;
                }
                $rSeq = substr(substr($pSeq, $pos) . substr($pSeq, 0, $pos), 0, $pSeqLen);
            }

            #Run tSeq and rSeq through Levenshtein to get original Edit Distance score
            $editDistanceScores{$h} = distance($origTSeq, $rSeq);

            print OF "Mode: $mode, Dissimilarity: $pc, Factor Length: $h\n";
            print OF "t = $origTSeq\np = $origPSeq\n";
            print OF "rp = $pSeq\n";
            print OF "Alignment: $cmdOutput\n";
            print OF "t = $origTSeq\nr = $rSeq\n";
            print OF "Edit Distance between tSeq and rSeq: $editDistanceScores{$h}\n";
            print OF "Completed MW_MaxShift search in $finish ms\n\n";
        }
        
        for my $key (keys(%editDistanceScores)) {
            my $score = $originalEditDistance - $editDistanceScores{$key};
            print OF "Factor length $key: $originalEditDistance - $editDistanceScores{$key} = $score\n";
        }

        close(OF);
    }
}

1;
