#
# Copyright 2015 Chris Lamb <lamby@debian.org>
#
# This file is part of strip-nondeterminism.
#
# strip-nondeterminism is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# strip-nondeterminism is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with strip-nondeterminism.  If not, see <http://www.gnu.org/licenses/>.
#
package File::StripNondeterminism::handlers::pearregistry;

use strict;
use warnings;

use File::StripNondeterminism;
use File::StripNondeterminism::Common qw(copy_data);
use File::Temp;
use File::Basename;

sub is_registry_file($) {
	my ($filename) = @_;

	# Registry files will always start with "a:"
	my $fh;
	my $str;
	return open($fh, '<', $filename) && read($fh, $str, 2) && $str =~ "^a:";
}

sub normalize {
	my ($filename) = @_;

	open(my $fh, '<', $filename)
	  or die "Unable to open $filename for reading: $!";

	my $modified = 0;
	my $tempfile = File::Temp->new(DIR => dirname($filename));
	my $canonical_time = $File::StripNondeterminism::canonical_time // 0;

	while (defined(my $line = <$fh>)) {
		# Normalize _lastmodified
		if ($line =~ s/(?<=s:13:"_lastmodified";i:)\d+(?=;)/$canonical_time/g)
		{
			$modified = 1;
		}

		print $tempfile $line;
	}

	if ($modified) {
		$tempfile->close;
		copy_data($tempfile->filename, $filename)
		  or die "$filename: unable to overwrite: copy_data: $!";
	}

	return $modified;
}

1;
