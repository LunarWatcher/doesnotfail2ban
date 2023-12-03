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

# TODO: conditionally link fmt (probably)
# Source library linking
target_link_libraries(dnf2b_src PUBLIC stc fmt::fmt nlohmann_json spdlog cpr::cpr)

# Executable linking
target_link_libraries(dnf2b PUBLIC dnf2b_src)

# Installation {{{
# TODO: Fix this crap
install(DIRECTORY ../etc/dnf2b ../etc/systemd DESTINATION /etc)

install(TARGETS dnf2b DESTINATION bin)
# }}}

# vim:ft=cmake
