#
# Copyright 2014 Andrew Ayer
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
package File::StripNondeterminism;

use strict;
use warnings;

use POSIX qw(tzset);

our($VERSION, $canonical_time, $clamp_time);

$VERSION = '0.040'; # 0.040

sub init() {
	$ENV{'TZ'} = 'UTC';
	tzset();
}

sub _get_file_type($) {
	my $file=shift;
	open(FILE, '-|') # handle all filenames safely
	  || exec('file', $file)
	  || die "can't exec file: $!";
	my $type=<FILE>;
	close FILE;
	return $type;
}

sub get_normalizer_for_file($) {
	$_ = shift;

	return undef if -d $_; # Skip directories

	# ar
	if (m/\.a$/ && _get_file_type($_) =~ m/ar archive/) {
		return _handler('ar');
	}
	# cpio
	if (m/\.cpio$/ && _get_file_type($_) =~ m/cpio archive/) {
		return _handler('cpio');
	}
	# gettext
	if (m/\.g?mo$/ && _get_file_type($_) =~ m/GNU message catalog/) {
		return _handler('gettext');
	}
	# gzip
	if (m/\.(gz|dz)$/ && _get_file_type($_) =~ m/gzip compressed data/) {
		return _handler('gzip');
	}
	# jar
	if (m/\.(jar|war|hpi|apk)$/
		&& _get_file_type($_) =~ m/(Java|Zip) archive data/) {
		return _handler('jar');
	}
	# javadoc
	if (m/\.html$/) {
		# Loading the handler forces the load of the javadoc package as well
		my $handler = _handler('javadoc');
		return $handler
		  if File::StripNondeterminism::handlers::javadoc::is_javadoc_file($_);
	}
	# pear registry
	if (m/\.reg$/) {
	  # Loading the handler forces the load of the pearregistry package as well
		my $handler = _handler('pearregistry');
		return $handler
		  if
		  File::StripNondeterminism::handlers::pearregistry::is_registry_file(
			$_);
	}
	# PNG
	if (m/\.png$/ && _get_file_type($_) =~ m/PNG image data/) {
		return _handler('png');
	}
	# pom.properties, version.properties
	if (m/\.properties$/) {
	# Loading the handler forces the load of the javaproperties package as well
		my $handler = _handler('javaproperties');
		return $handler
		  if
		  File::StripNondeterminism::handlers::javaproperties::is_java_properties_file(
			$_);
	}
	# zip
	if (m/\.(zip|pk3|epub|whl|xpi|htb|zhfst|par)$/
		&& _get_file_type($_) =~ m/Zip archive data|EPUB document/) {
		return _handler('zip');
	}
	return undef;
}

our %HANDLER_CACHE;
our %KNOWN_HANDLERS = (
	ar	=> 1,
	cpio	=> 1,
	gettext	=> 1,
	gzip	=> 1,
	jar	=> 1,
	javadoc	=> 1,
	pearregistry => 1,
	png	=> 1,
	javaproperties => 1,
	zip	=> 1,
);

sub _handler {
	my ($handler_name) = @_;
	return $HANDLER_CACHE{$handler_name}
	  if exists($HANDLER_CACHE{$handler_name});
	die("Unknown handler: ${handler_name}\n")
	  if not exists($KNOWN_HANDLERS{$handler_name});
	my $pkg = "File::StripNondeterminism::handlers::${handler_name}";
	my $mod = "File/StripNondeterminism/handlers/${handler_name}.pm";
	my $sub_name = "${pkg}::normalize";
	require $mod;
	no strict 'refs';

	if (not defined &{$sub_name}) {
		die("Internal error: No handler for $handler_name!?\n");
	}
	my $handler = \&{$sub_name};
	return $HANDLER_CACHE{$handler_name} = $handler;
}

sub get_normalizer_by_name($) {
	return _handler(shift);
}

1;
