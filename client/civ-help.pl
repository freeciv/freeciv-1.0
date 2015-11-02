#!/usr/local/bin/perl5 -w

use strict;

die "Usage: $0 <civ-help file> <helptab c-file>\n" if @ARGV != 2;

open HELP, $ARGV[0] or die "$ARGV[0]: $!\n";

open TAB, "> $ARGV[1]" or die "> $ARGV[1]: $!\n";

print TAB "char *help_indexlist[] = {\n";

while (<HELP>) {		# Read and print out the list
    last if /^--/;

    chomp;
    /\s+$/;			# Kill whitespace at end of line
    print TAB <<"EOS";
  "$_",
EOS
}

print TAB <<"EOS";
  NULL
};

char *help_items[] = {
EOS

while (<HELP>) {
    chomp;
    if (/^--/) {
	print TAB ",\n";
	next;
    }
    else {
	print TAB "\n";
    }

    /\s+$/;			# Kill whitespace at end of line

    print TAB "\"$_\\n\"";
}

print TAB "};\n";

close TAB;
close HELP;

