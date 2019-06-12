
add_library(Qt5::WorldTimeClockPlugin MODULE IMPORTED)

_populate_UiTools_plugin_properties(WorldTimeClockPlugin RELEASE "designer/libworldtimeclockplugin.so")

list(APPEND Qt5UiTools_PLUGINS Qt5::WorldTimeClockPlugin)
