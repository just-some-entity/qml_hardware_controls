qt_add_executable(the_test
        the_test.cpp
        ../brightness.cpp
        ../hardware_manager.cpp
        ../collection/cpu_collector.cpp
        ../samplers/cpu_sampler_simple.cpp
)

qt_add_resources(the_test "test_resources"
        PREFIX "/tests/primary"
        FILES
            test.qml
)

target_include_directories(the_test PRIVATE ${LOGIND_COMPAT_INCLUDE_DIRS})
target_link_libraries(the_test PRIVATE Qt6::Quick Qt6::Gui Qt::Core Qt::Qml ${LOGIND_COMPAT_LIBRARIES})
target_compile_options(the_test PRIVATE ${LOGIND_COMPAT_CFLAGS_OTHER})