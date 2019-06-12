require '_h2ph_pre.ph';

no warnings qw(redefine misc);

unless(defined(&_BITS_PTHREADTYPES_ARCH_H)) {
    eval 'sub _BITS_PTHREADTYPES_ARCH_H () {1;}' unless defined(&_BITS_PTHREADTYPES_ARCH_H);
    require 'endian.ph';
    eval 'sub __SIZEOF_PTHREAD_ATTR_T () {36;}' unless defined(&__SIZEOF_PTHREAD_ATTR_T);
    eval 'sub __SIZEOF_PTHREAD_MUTEX_T () {24;}' unless defined(&__SIZEOF_PTHREAD_MUTEX_T);
    eval 'sub __SIZEOF_PTHREAD_MUTEXATTR_T () {4;}' unless defined(&__SIZEOF_PTHREAD_MUTEXATTR_T);
    eval 'sub __SIZEOF_PTHREAD_COND_T () {48;}' unless defined(&__SIZEOF_PTHREAD_COND_T);
    eval 'sub __SIZEOF_PTHREAD_CONDATTR_T () {4;}' unless defined(&__SIZEOF_PTHREAD_CONDATTR_T);
    eval 'sub __SIZEOF_PTHREAD_RWLOCK_T () {32;}' unless defined(&__SIZEOF_PTHREAD_RWLOCK_T);
    eval 'sub __SIZEOF_PTHREAD_RWLOCKATTR_T () {8;}' unless defined(&__SIZEOF_PTHREAD_RWLOCKATTR_T);
    eval 'sub __SIZEOF_PTHREAD_BARRIER_T () {20;}' unless defined(&__SIZEOF_PTHREAD_BARRIER_T);
    eval 'sub __SIZEOF_PTHREAD_BARRIERATTR_T () {4;}' unless defined(&__SIZEOF_PTHREAD_BARRIERATTR_T);
    eval 'sub __PTHREAD_COMPAT_PADDING_MID () {1;}' unless defined(&__PTHREAD_COMPAT_PADDING_MID);
    eval 'sub __PTHREAD_COMPAT_PADDING_END () {1;}' unless defined(&__PTHREAD_COMPAT_PADDING_END);
    eval 'sub __PTHREAD_MUTEX_LOCK_ELISION () {0;}' unless defined(&__PTHREAD_MUTEX_LOCK_ELISION);
    eval 'sub __PTHREAD_MUTEX_NUSERS_AFTER_KIND () {1;}' unless defined(&__PTHREAD_MUTEX_NUSERS_AFTER_KIND);
    eval 'sub __PTHREAD_MUTEX_USE_UNION () {1;}' unless defined(&__PTHREAD_MUTEX_USE_UNION);
    eval 'sub __LOCK_ALIGNMENT () {1;}' unless defined(&__LOCK_ALIGNMENT);
    eval 'sub __ONCE_ALIGNMENT () {1;}' unless defined(&__ONCE_ALIGNMENT);
    if((defined(&__BYTE_ORDER) ? &__BYTE_ORDER : undef) == (defined(&__BIG_ENDIAN) ? &__BIG_ENDIAN : undef)) {
    } else {
    }
    eval 'sub __PTHREAD_RWLOCK_ELISION_EXTRA () {0;}' unless defined(&__PTHREAD_RWLOCK_ELISION_EXTRA);
}
1;
