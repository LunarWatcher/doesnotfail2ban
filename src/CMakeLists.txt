set (SOURCE_FILES
    dnf2b/core/Context.cpp
    dnf2b/core/Daemon.cpp

    dnf2b/sources/Parser.cpp
    dnf2b/sources/FileParser.cpp

    dnf2b/ui/CLI.cpp
)

add_library(dnf2b_src STATIC ${SOURCE_FILES})

add_executable(dnf2b dnf2b/Main.cpp)
target_include_directories(dnf2b_src PUBLIC ./)
target_include_directories(dnf2b_src PUBLIC ${CONAN_INCLUDE_DIRS})

# Source library linking
target_link_libraries(dnf2b_src PUBLIC ${CONAN_LIBS} nlohmann_json spdlog)

# Executable linking
target_link_libraries(dnf2b PUBLIC dnf2b_src)

# Installation {{{
install(DIRECTORY ../etc/dnf2b ../etc/systemd DESTINATION /etc)

# Now, the cool thing about CMake is that the destination is relative to --prefix
install(TARGETS dnf2b DESTINATION bin)
# }}}

# vim:ft=cmake
