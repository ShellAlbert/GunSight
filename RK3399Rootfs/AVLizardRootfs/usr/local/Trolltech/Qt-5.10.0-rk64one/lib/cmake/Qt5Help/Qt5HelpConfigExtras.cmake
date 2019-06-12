
if (NOT TARGET Qt5::qcollectiongenerator)
    add_executable(Qt5::qcollectiongenerator IMPORTED)

    set(imported_location "/usr/local/Trolltech/Qt-5.10.0-rk64one-sdk/bin/qcollectiongenerator")
    _qt5_Help_check_file_exists(${imported_location})

    set_target_properties(Qt5::qcollectiongenerator PROPERTIES
        IMPORTED_LOCATION ${imported_location}
    )
endif()

if (NOT TARGET Qt5::qhelpgenerator)
    add_executable(Qt5::qhelpgenerator IMPORTED)

    set(imported_location "/usr/local/Trolltech/Qt-5.10.0-rk64one-sdk/bin/qhelpgenerator")
    _qt5_Help_check_file_exists(${imported_location})

    set_target_properties(Qt5::qhelpgenerator PROPERTIES
        IMPORTED_LOCATION ${imported_location}
    )
endif()
