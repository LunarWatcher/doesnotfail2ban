cmake_minimum_required(VERSION 3.21)
project(doesnotfail2ban)

set (CMAKE_CXX_STANDARD 20)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
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
set(BUILD_SHARED_LIBS ON CACHE STRING "" FORCE)

# Find the systemd library
find_package(PkgConfig REQUIRED)
pkg_check_modules(SYSTEMD "systemd" REQUIRED)
pkg_check_modules(JOURNALD "libsystemd" REQUIRED)

find_path(SYSTEMD_INCLUDE_DIR
    NAMES systemd/sd-journal.h
    HINTS ${PC_SYSTEMD_INCLUDE_DIRS})
find_library(SYSTEMD_LIBRARY
    NAMES systemd
    HINTS ${PC_SYSTEMD_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SYSTEMD
    REQUIRED_VARS
        SYSTEMD_LIBRARY
        SYSTEMD_INCLUDE_DIR)

mark_as_advanced(SYSTEMD_INCLUDE_DIR SYSTEMD_LIBRARY)

set(SYSTEMD_INCLUDE_DIRS ${SYSTEMD_INCLUDE_DIR})
set(SYSTEMD_LIBRARIES ${SYSTEMD_LIBRARY})


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
FetchContent_Declare(sql
    GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp
    GIT_TAG 3.3.1)
FetchContent_Declare(pcre
    GIT_REPOSITORY https://github.com/PCRE2Project/pcre2
    GIT_TAG pcre2-10.42)
FetchContent_Declare(cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11
    GIT_TAG v2.3.2)

FetchContent_MakeAvailable(cli11)
#FetchContent_MakeAvailable(fmt)
FetchContent_MakeAvailable(spdlog)
FetchContent_MakeAvailable(nlohmann)
FetchContent_MakeAvailable(stc)

FetchContent_GetProperties(fmt)
if (NOT fmt_POPULATED)
    FetchContent_Populate(fmt)
    add_subdirectory(${fmt_SOURCE_DIR} ${fmt_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_GetProperties(pcre)
if (NOT pcre_POPULATED)
    FetchContent_Populate(pcre)
    add_subdirectory(${pcre_SOURCE_DIR} ${pcre_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_GetProperties(sql)
if (NOT sql_POPULATED)
    FetchContent_Populate(sql)
    add_subdirectory(${sql_SOURCE_DIR} ${sql_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_GetProperties(cpr)
if (NOT cpr_POPULATED)
    FetchContent_Populate(cpr)
    add_subdirectory(${cpr_SOURCE_DIR} ${cpr_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

option(ASIO_USE_CPM "Download Asio with CPM instead of FetchContent" OFF)
option(
  ASIO_CPM_FETCHCONTENT_COMPAT
  "Should asio be declared with FetchContent functions to be compatible. This doesn't not allow CPM cache to work."
  ON
)
# Download this repository
include(FetchContent)
FetchContent_Declare(
  asiocmake
  GIT_REPOSITORY "https://github.com/OlivierLDff/asio.cmake"
  GIT_TAG        "main"
)
FetchContent_MakeAvailable(asiocmake)

# Set up versioning {{{
find_program(GIT git)
if (GIT)
    execute_process(COMMAND ${GIT} describe --tags --always
        OUTPUT_VARIABLE DNF2B_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "Detected version: ${DNF2B_VERSION}")
else()
    message(WARNING "Git not found. Defaulting to unknown version. This will not have an effect on the program beyond a missing version")
    set(DNF2B_VERSION "UNKNOWN")
endif()
add_definitions(-DDNF2B_VERSION="${DNF2B_VERSION}")
# }}}

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -fsanitize=undefined")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wextra")
add_definitions(-DPCRE2_CODE_UNIT_WIDTH=8)

if (ENABLE_COVERAGE)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -coverage")

    #target_link_options(-lgcov --coverage)
    add_custom_target(coverage
        COMMAND lcov --directory src --capture --output-file coverage.info
        COMMAND lcov -r coverage.info '/usr*' '${CMAKE_BINARY_DIR}/_deps/*' -o coverage.info
        COMMAND genhtml --demangle-cpp -o coverage coverage.info
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

add_subdirectory(src)
add_subdirectory(tests EXCLUDE_FROM_ALL)

add_custom_target(test
    COMMAND tests
    DEPENDS tests
    COMMENT "Run tests")

# vim:ft=cmake
