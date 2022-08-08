cmake_minimum_required(VERSION 3.10)
project(doesnotfail2ban)

set (CMAKE_CXX_STANDARD 20)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
#  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
#  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.16.1/conan.cmake"
#                "${CMAKE_BINARY_DIR}/conan.cmake"
#                TLS_VERIFY ON)
#endif()
#
#include(${CMAKE_BINARY_DIR}/conan.cmake)
#
#conan_cmake_configure (REQUIRES 
#                        insert dependencies here
#                        GENERATORS cmake)
#conan_cmake_autodetect (settings)
#conan_cmake_install (PATH_OR_REFERENCE . BUILD missing SETTINGS ${settings})
#
#include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
#conan_basic_setup()

add_subdirectory(src)
add_subdirectory(tests)
# vim:ft=cmake
