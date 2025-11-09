qml_module(qml_hardware_controls
        URI
            HardwareControls

        SOURCES
            brightness.cpp

        LIBRARIES
            Qt6::Core
            Qt6::Quick
            ${LOGIND_COMPAT_LIBRARIES}

        INCLUDES
            ${LOGIND_COMPAT_INCLUDE_DIRS}

        CFLAGS
            ${LOGIND_COMPAT_CFLAGS_OTHER}
)

add_subdirectory(tests)