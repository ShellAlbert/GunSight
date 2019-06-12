host_build {
    QT_ARCH = arm64
    QT_BUILDABI = arm64-little_endian-lp64
    QT_TARGET_ARCH = arm64
    QT_TARGET_BUILDABI = arm64-little_endian-lp64
} else {
    QT_ARCH = arm64
    QT_BUILDABI = arm64-little_endian-lp64
}
QT.global.enabled_features = cross_compile shared rpath c++11 c++14 c++1z concurrent
QT.global.disabled_features = framework appstore-compliant debug_and_release simulator_and_device build_all pkg-config force_asserts separate_debug_info static
CONFIG += cross_compile shared release
QT_CONFIG += shared rpath release c++11 c++14 c++1z concurrent dbus no-pkg-config no-qml-debug reduce_exports release_tools stl
QT_VERSION = 5.10.0
QT_MAJOR_VERSION = 5
QT_MINOR_VERSION = 10
QT_PATCH_VERSION = 0
QT_GCC_MAJOR_VERSION = 7
QT_GCC_MINOR_VERSION = 3
QT_GCC_PATCH_VERSION = 0
QT_EDITION = OpenSource
