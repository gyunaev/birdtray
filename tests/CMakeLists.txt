cmake_minimum_required(VERSION 3.10)

find_package(GTest QUIET)
if (NOT GTEST_FOUND)
    message("Unable to find GTest on the system, downloading it...")
    add_subdirectory(deps/GTest)
endif()


set(TESTS
        src/test_morkparser.cpp
        )

add_executable(tests src/TestResources.cpp src/TestResources.h ${TESTS})
target_include_directories(tests PRIVATE src)
target_link_libraries(tests GTest::GTest GMock::GMock GMock::Main birdtray_lib)
gtest_discover_tests(tests)

add_custom_target(run_tests COMMENT "Run the Birdtray tests")
add_custom_command(TARGET run_tests
        POST_BUILD COMMAND $<TARGET_FILE:tests>)
