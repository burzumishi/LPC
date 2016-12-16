#!/usr/bin/perl

use strict;
use warnings FATAL=>'all';

opendir(A,'.');
my @topdir = readdir(A);
close A;
for my $b (@topdir) {
	next if $b =~ /^\./;
	next unless -d $b;
	opendir(A,$b);
	my @curdir = readdir(A);
	my $d;
	chdir $b;
	for my $c (@curdir) {
		next if $c =~ /^\./;
		$d = $c . '.tmp';	
		open(F,$c);
		open(G,">$d");
		while(<F>) {
			s|/d/Genesis|/d/Standard|g;
			print G $_;
		}
		close G;
		close F;
		rename $d,$c;
	}
	closedir A;
	chdir '..';
}
