qml_module(qml_hardware_controls
        URI
            HardwareControls

        SOURCES
            hardware_manager.cpp
            brightness.cpp
            collection/cpu_collector.cpp

            samplers/cpu_sampler_simple.h
            samplers/cpu_sampler_simple.cpp

        LIBRARIES
            Qt6::Core
            Qt6::Quick
            ${LOGIND_COMPAT_LIBRARIES}

        INCLUDES
            ${LOGIND_COMPAT_INCLUDE_DIRS}
            ${CMAKE_CURRENT_SOURCE_DIR}/samplers

        CFLAGS
            ${LOGIND_COMPAT_CFLAGS_OTHER}
)

add_subdirectory(tests)