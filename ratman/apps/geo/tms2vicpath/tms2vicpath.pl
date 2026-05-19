#!/usr/bin/perl

sub convert() {
    my $path_in=$_;
    my $result=$path_in;
    if((my $basepath,my $level,my $x,my $y,my $ext)=/(^.*\/)(\d+)\/(\d+)\/(\d+)\.(\w*)$/) {
	my $xhh=($x/262144);
	my $xlh = ($x/512)%512;
	my $xll = ($x%512);
	my $yhh = ($y/262144);
	my $ylh = ($y/512)%512;
	my $yll = ($y%512);
	$result=sprintf("%s%02d/%03d/%03d/%03d/%03d/%03d/%03d.%s",$basepath,
			$level,
			$xhh, $xlh, $xll,
			$yhh, $ylh, $yll,
			$ext);			
    }
    $result;
}




#   disable buffered I/O which would lead
#   to deadloops for the Apache server
$| = 1;

#   read URLs one per line from stdin and
#   generate substitution URL on stdout
while (<>) {
    my $result=&convert($_);
    print "$result\n";
}
