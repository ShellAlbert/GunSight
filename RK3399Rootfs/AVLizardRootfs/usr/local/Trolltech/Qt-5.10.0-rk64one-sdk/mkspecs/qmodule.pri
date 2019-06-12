host_build {
    QT_CPU_FEATURES.arm64 = neon
} else {
    QT_CPU_FEATURES.arm64 = neon
}
QT.global_private.enabled_features = alloca_h alloca dbus gui libudev network posix_fallocate reduce_exports release_tools sql testlib widgets xml
QT.global_private.disabled_features = sse2 alloca_malloc_h android-style-assets avx2 dbus-linked private_tests qml-debug reduce_relocations stack-protector-strong system-zlib
QMAKE_LIBS_LIBUDEV = -ludev
QT_COORD_TYPE = double
CONFIG += cross_compile use_gold_linker compile_examples enable_new_dtags largefile neon precompile_header
QT_BUILD_PARTS += tools libs
QT_HOST_CFLAGS_DBUS += -I/usr/include/dbus-1.0 -I/usr/lib/aarch64-linux-gnu/dbus-1.0/include
