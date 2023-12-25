option(
    INTEGRATION
    "Whether or not to enable integration tests. Disable by default as these may write to a systemd journal, which is notoriously difficult to clean up"
    OFF
)

add_executable(tests src/TestRunner.cpp

    src/util/MockHelper.cpp
    # Tests {{{
    src/ParserTests.cpp
    src/WatcherTests.cpp
    src/BanManagerTests.cpp
    src/DatabaseTests.cpp
    src/ParsingUtilTests.cpp
    src/PCRETests.cpp
    src/FilterTests.cpp
    src/MessageBufferTests.cpp

    src/regression/ConfigRegressionTests.cpp

    src/integration/JournaldParserTests.cpp
    # }}}
)
target_include_directories(tests PUBLIC src/)

# Catch2 {{{
Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.0)

FetchContent_MakeAvailable(Catch2)

target_link_libraries(tests Catch2::Catch2)
# }}}

add_custom_target(test_resources
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/test-files

    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/etc ${CMAKE_BINARY_DIR}/test-files
)


add_dependencies(tests test_resources)

target_link_libraries(tests dnf2b_src)

if (INTEGRATION)
    message(INFO "Enabling integration tests")

    target_compile_definitions(
        tests
    PUBLIC
        INTEGRATION_TESTS=1
    )
endif()
