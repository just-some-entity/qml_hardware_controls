cmake_minimum_required(VERSION 4.0)
project(qml_hardware_controls)

set(VERSION 1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

set(QT_QML_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml")
set(INSTALL_QMLDIR "/usr/lib/qt6/qml")

# Most systems have logind so set this to ON
option(ENABLE_LOGIND "Enable systemd-logind support" ON)

include(${CMAKE_SOURCE_DIR}/cmake/utils.cmake)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Qml Gui)

set(LOGIND_COMPAT_INCLUDE_DIRS "")
set(LOGIND_COMPAT_LIBRARIES "")
set(LOGIND_COMPAT_CFLAGS_OTHER "")

if(ENABLE_LOGIND)
    add_compile_definitions(ENABLE_LOGIND)

    find_package(PkgConfig REQUIRED)

    pkg_check_modules(SYSTEMD libsystemd)

    if(SYSTEMD_FOUND)
        message(STATUS "Found systemd, using LIBSYSTEMD for D-Bus")

        add_compile_definitions(HAVE_LIBSYSTEMD)

        message(STATUS "includes   ${SYSTEMD_INCLUDE_DIRS}")
        message(STATUS "libraries  ${SYSTEMD_LIBRARIES}")

        set(LOGIND_COMPAT_INCLUDE_DIRS ${SYSTEMD_INCLUDE_DIRS})
        set(LOGIND_COMPAT_LIBRARIES    ${SYSTEMD_LIBRARIES})
        set(LOGIND_COMPAT_CFLAGS_OTHER ${SYSTEMD_CFLAGS_OTHER})
    else()
        pkg_check_modules(ELOGIND libelogind)
        if(ELOGIND_FOUND)
            message(STATUS "Found elogind, using LIBELOGIND for D-Bus")
            add_compile_definitions(HAVE_LIBELOGIND)
            set(LOGIND_COMPAT_INCLUDE_DIRS ${ELOGIND_INCLUDE_DIRS})
            set(LOGIND_COMPAT_LIBRARIES    ${ELOGIND_LIBRARIES})
            set(LOGIND_COMPAT_CFLAGS_OTHER ${ELOGIND_CFLAGS_OTHER})
        else()
            pkg_check_modules(BASU basu)
            if(BASU_FOUND)
                message(STATUS "Using BASU for D-Bus")
                add_compile_definitions(HAVE_BASU)
                set(LOGIND_COMPAT_INCLUDE_DIRS  ${BASU_INCLUDE_DIRS})
                set(LOGIND_COMPAT_LIBRARIES     ${BASU_LIBRARIES})
                set(LOGIND_COMPAT_CFLAGS_OTHER  ${BASU_CFLAGS_OTHER})
            endif()
        endif()
    endif()
endif()

qt_standard_project_setup(REQUIRES 6.6)

add_subdirectory("src")

if(NOT DEFINED ENABLE_LOGIND)
    if(NOT DEFINED UDEVDIR)
        set(UDEVDIR "/lib/udev/rules.d" CACHE PATH "Directory for udev rules")
    endif()

    # Install the udev rules file
    install(FILES 90-hardware-controls-brightness.rules
            DESTINATION "${UDEVDIR}"
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
endif()