qt_add_executable(brightness_test
        ../brightness.cpp
        brightness_test.cpp)

qt_add_resources(brightness_test "test_resources"
        PREFIX "/tests/brightness"
        FILES
            test.qml
)

target_include_directories(brightness_test PRIVATE ${LOGIND_COMPAT_INCLUDE_DIRS})
target_link_libraries(brightness_test PRIVATE Qt6::Quick Qt6::Gui Qt::Core Qt::Qml ${LOGIND_COMPAT_LIBRARIES})
target_compile_options(brightness_test PRIVATE ${LOGIND_COMPAT_CFLAGS_OTHER})