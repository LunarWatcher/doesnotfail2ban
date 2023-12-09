add_executable(tests src/TestRunner.cpp

    src/util/MockHelper.cpp
    # Tests {{{
    src/ParserTest.cpp
    src/WatcherTests.cpp
    src/BanManagerTests.cpp
    src/DatabaseTests.cpp
    src/ParsingUtilTests.cpp
    # }}}
)

# Catch2 {{{
Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.1.0)

FetchContent_MakeAvailable(Catch2)

target_link_libraries(tests Catch2::Catch2)
# }}}

add_custom_target(test_resources
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/test-files

    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/etc ${CMAKE_BINARY_DIR}/test-files
)

add_dependencies(tests test_resources)

target_link_libraries(tests dnf2b_src)
