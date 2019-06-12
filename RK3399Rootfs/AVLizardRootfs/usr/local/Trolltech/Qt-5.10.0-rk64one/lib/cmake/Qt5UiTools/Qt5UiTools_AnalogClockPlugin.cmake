
add_library(Qt5::AnalogClockPlugin MODULE IMPORTED)

_populate_UiTools_plugin_properties(AnalogClockPlugin RELEASE "designer/libcustomwidgetplugin.so")

list(APPEND Qt5UiTools_PLUGINS Qt5::AnalogClockPlugin)
