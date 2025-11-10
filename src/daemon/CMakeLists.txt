qt_add_executable(daemon daemon.cpp)

target_include_directories(daemon PRIVATE ${LOGIND_COMPAT_INCLUDE_DIRS})
target_link_libraries(daemon PRIVATE Qt6::Quick Qt6::Gui Qt::Core Qt::Qml ${LOGIND_COMPAT_LIBRARIES})
target_compile_options(daemon PRIVATE ${LOGIND_COMPAT_CFLAGS_OTHER})