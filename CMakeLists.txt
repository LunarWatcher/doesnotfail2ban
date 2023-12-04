cmake_minimum_required(VERSION 3.10)
project(doesnotfail2ban)

set (CMAKE_CXX_STANDARD 20)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(USE_LIBCPP)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
endif()

# Control varaible used to control excessively verbose messages. Logging wrapped in a
# #ifdef for this variable need to be excessively verbose, and not useful outside
# development debugging.
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDNF2B_VERY_VERBOSE")

set(SPDLOG_FMT_EXTERNAL ON CACHE STRING "" FORCE)
set(CURL_ZLIB ON CACHE STRING "" FORCE)

include(FetchContent)
FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 9.1.0)
FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG v1.11.0)
FetchContent_Declare(nlohmann
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG v3.11.2)
FetchContent_Declare(cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.10.0)
FetchContent_Declare(stc
    GIT_REPOSITORY https://github.com/LunarWatcher/stc)

FetchContent_MakeAvailable(cpr)
FetchContent_MakeAvailable(fmt)
FetchContent_MakeAvailable(spdlog)
FetchContent_MakeAvailable(nlohmann)
FetchContent_MakeAvailable(stc)

add_subdirectory(src)
add_subdirectory(tests EXCLUDE_FROM_ALL)

add_custom_target(test
    COMMAND tests
    DEPENDS tests
    COMMENT "Run tests")

# vim:ft=cmake
