#! /usr/bin/perl
use strict;
use warnings;

my @lengthTags = qw(8 16 32 64);
my @valueTypes = qw(uint8_t uint16_t uint32_t uint64_t);
my @accumTypes = qw(uint_fast32_t uint_fast32_t uint_fast32_t uint_fast64_t);

my @inputs = @ARGV;

foreach my $input (@inputs)
{
    open(INPUT, '<', $input)
        or die('Failed to open ', $input, ' for reading: ');
    for(my $i = 0; $i < @lengthTags; ++$i)
    {
        my $output = $input;
        $output =~ s/\.template$/$lengthTags[$i]\.c/;
        open(OUTPUT, '>', $output)
            or die('Failed to open ', $output, ' for writing: ');
        while(<INPUT>)
        {
            s/\@LEN\@/$lengthTags[$i]/g;
            s/\@ValueType\@/$valueTypes[$i]/g;
            s/\@AccumType\@/$accumTypes[$i]/g;
            print(OUTPUT $_);
        }
        close(OUTPUT);
        seek(INPUT, 0, 0);
    }
}
