GENCCODE_ASSEMBLY_TYPE=-a gcc
SO=so
SOBJ=so
A=a
LIBPREFIX=lib
LIB_EXT_ORDER=.60.2
COMPILE=gcc -Wdate-time -D_FORTIFY_SOURCE=2 -D_REENTRANT  -DU_HAVE_ELF_H=1 -DU_HAVE_ATOMIC=1 -DU_HAVE_STRTOD_L=1 -DU_HAVE_XLOCALE_H=0  -DU_ATTRIBUTE_DEPRECATED= -g -O2 -fdebug-prefix-map=/build/icu-m51M4s/icu-60.2=. -fstack-protector-strong -Wformat -Werror=format-security -std=c99 -Wall -pedantic -Wshadow -Wpointer-arith -Wmissing-prototypes -Wwrite-strings   -c
LIBFLAGS=-I/usr/include -DPIC -fPIC
GENLIB=gcc -g -O2 -fdebug-prefix-map=/build/icu-m51M4s/icu-60.2=. -fstack-protector-strong -Wformat -Werror=format-security -std=c99 -Wall -pedantic -Wshadow -Wpointer-arith -Wmissing-prototypes -Wwrite-strings   -Wl,-Bsymbolic-functions -Wl,-z,relro -Wl,-z,now  -shared -Wl,-Bsymbolic
LDICUDTFLAGS=
LD_SONAME=-Wl,-soname -Wl,
RPATH_FLAGS=
BIR_LDFLAGS=-Wl,-Bsymbolic
AR=ar
ARFLAGS=r
RANLIB=ranlib
INSTALL_CMD=/usr/bin/install -c
