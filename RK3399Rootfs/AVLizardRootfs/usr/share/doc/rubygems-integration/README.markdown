# rubygems-integration

This package makes Debian Ruby recognizable by Rubygems. The packages will be
listed by `gem list` and will be recognized by Rubygems as being installed.
They can be used to satisfy dependencies both with plain Rubygems and with
Bundler. As a special case, they can be used to satisfy dependencies of Rails 3
applications.

# How to make a Debian Ruby package work with rubygems-integration

Install a gemspec file to
/usr/share/rubygems-integration/$VERSION/specifications to make the package
available for Ruby $VERSION (e.g. if $VERSION is 1.9.1 the the package will be
visible to Rubygems on Ruby 1.9)

For packages that should work with any Ruby version, you must install the
gemspec to /usr/share/rubygems-integration/all/specifications instead.

`dh_ruby(1)` from the `gem2deb` package will do the above automatically for
you.

# Disabling rubygems-integration

Even though rubygems-integration is insanely useful, there might be some cases
where you want to disable it. To do that, export a environment variable named
`DEBIAN_DISABLE_RUBYGEMS_INTEGRATION`. Only the presence of this variable
matters, its value is completely ignored.

# Copyright

Copyright © 2012-2014 Antonio Terceiro <terceiro@debian.org>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
