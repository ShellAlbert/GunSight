require '_h2ph_pre.ph';

no warnings qw(redefine misc);

unless(defined(&__NO_LONG_DOUBLE_MATH)) {
    eval 'sub __NO_LONG_DOUBLE_MATH () {1;}' unless defined(&__NO_LONG_DOUBLE_MATH);
}
1;
