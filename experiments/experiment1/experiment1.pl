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

my $exePath = "../../dist/";
my $exe = "mw_maxshift";

#2000 chars
my $t = <<'SEQ';
tcttttcttatcgttgacatttaaactctggggcaggtcctcgcgtagaa
cccggttgtcagatctgctacttccccccgagaagcggctttgagaagtg
tgggaaccagcgccaccagactcacctgacaccccaccctcggctcacag
gtaaccgctttgattctagccagactcaccccaagagtagcggcgcccct
ctccgacgtgcaacctgtgtgttcaggtcatagaaaccctccccgagtgg
ccccggccttgatttttttctttttctttttaaaggaagcttacaaagat
ggtagaggagttgctgcggatgggacgggggtgagggggaggggatgtct
ttgcgaagcatgcttctttgtaaaagtaacaaaatgttgtggtctcaaga
gaggacttcccgtctggccctgagctgggggtgatgtggggggtgggagg
gtggtggctgtggccgcagtctaaagaaatcgctgagatcctaagaaaag
caggcggtgggcatgcagtttgcaacacgccttcccccgatgcaattagc
gacattcatgcttggagaagccaccaagcctcgcgccgaattcatggagc
accccgcggaaggtcgggcgacttgcgacaaaagtttccccaaggttgtg
tagatgtgtgttccatatcgaagtcaaagtcttggggttcgtttcccccg
aaaggagcacaggacgaaagaatgcggccctccagagctgggcagcgcgc
gcgaaggccccagcgtgtgatttgagcttccttcggaagacctaataatt
agcgattctcactgagctagaatgcgggctcgggttaccgggaacgggtt
cctagcagcggcggagctctttgcctcggcgggaagcatgtgggctccag
gggaacccggttgctgagtgccctggagagagatacccagccatgctccg
gctccaggaaggccagaacttggccgaagtgggtcaggaaagtgattacc
cctaacttaaactaaaaggtgttttcgccaggttggaaaatctctcactc
aagccctaagcttaccaggagccgctcaggctgcctgagtcggggcaccg
gagcaacccaccggaaatttgaggccatgtagcaggcctggcctggatct
cgcccactcccacgcgtgctcctgacgccctctctctagacccgctggag
gcacttgtccagatcgtttggctctcaagcacctgtgacaggtgcccagg
agtctaactctgagcagagtcccctctgagcgcctcacatagaaaggagg
ccacgggcaaagtggccaccagtctgagcaccttacctgaacgggggtcc
tctcctccgagagttccttattccagagttgagccgtgagcagtttgccc
gcttcgcagtaggaagtggaccttctcctctagtcataaatcaaaccagc
catcctcgggcctcctccctcattagagatgtttattggagattgtgttt
attcggctgtcacggcgagaaaacgcggtgacataattacctctgaccag
agtcctcgccctgcgcccagggtgagccagggacttcctctgctgtggtc
ttactctgctgccctccttgatcagagggcaaatcggattggtgtgtcta
ctttttcctccctcagacttcgtatcatttgaattattggagaacttagt
attcaaatttccgccagtgcccagacttggaaaagaaaaccaaagagaat
ccagcttccagggagagagggagagagaagtaagggagagagagagggaa
agggagagagagagagagagagagagagagagagagagagagagagagag
agtaaatcttgcatgagaactttctagtgcactggcagagatgcttggcc
agacctagaaggacgagtgcctgtatttgtggtattattttttcttttgc
ttccctttttcattccttttcttttcgggaggaatctggctgcagagcct
SEQ

$t =~ s/\r?\n//g;

#pattern to search for - 1536 chars from an arbitrary position
my $p = substr($t, 36, 1536);

#factor lengths
my @hlens = (8, 16, 32, 64, 128, 256, 512, 768, 1024, 1280, 1536);

#modes
my @modes = ("edls", "hdls"); #"ed", "hd"

#repeats if desired
my $repeats = 1;

for my $mode (@modes) {
    for my $h (@hlens) {
        print "Started MW_MaxShift $mode mode with factor length: $h\n";
        my $cmd = "$exePath$exe -m $mode -t $t -p $p -h $h";
        my $start = time();
        for (my $i = 0; $i < $repeats; $i++) {
            system($cmd);
        }
        my $finish = (time() - $start) * 1000;
        print "Completed MW_MaxShift $mode mode in $finish ms\n\n";
    }
}

1;
