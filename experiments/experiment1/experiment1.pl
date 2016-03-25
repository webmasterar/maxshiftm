#!/usr/bin/perl

# MaxShiftM
# Copyright (C) 2015 Solon Pissis and Ahmad Retha
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

use strict;
use warnings;
use Time::HiRes qw(time);

my $exePath = "../../";
my $exe = "maxshiftm";

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
my @modes = ("hdls"); #"hd"

#repeats if desired
my $repeats = 1;

for my $mode (@modes) {
    for my $h (@hlens) {
        print "Started MaxShiftM $mode mode with factor length: $h\n";
        my $cmd = "$exePath$exe -m $mode -t $t -p $p -h $h";
        my $start = time();
        for (my $i = 0; $i < $repeats; $i++) {
            system($cmd);
        }
        my $finish = (time() - $start) * 1000;
        print "Completed MaxShiftM $mode mode in $finish ms\n\n";
    }
}

1;

