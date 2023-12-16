set (SOURCE_FILES
    dnf2b/core/Context.cpp
    dnf2b/core/Daemon.cpp

    dnf2b/data/BanManager.cpp
    dnf2b/data/BanDB.cpp
    dnf2b/data/ReadStateDB.cpp

    dnf2b/sources/Parser.cpp
    dnf2b/sources/FileParser.cpp
    dnf2b/sources/JournalCTLParser.cpp
    dnf2b/sources/ParserLoader.cpp

    dnf2b/bouncers/BouncerLoader.cpp
    dnf2b/bouncers/IPTableBouncer.cpp

    dnf2b/watcher/Watcher.cpp

    dnf2b/filters/Filter.cpp

    dnf2b/util/Parsing.cpp
    dnf2b/util/PCRE.cpp
)
list(APPEND CMAKE_INSTALL_RPATH "$ORIGIN/../lib")

add_library(dnf2b_src STATIC ${SOURCE_FILES})

add_executable(dnf2b dnf2b/Main.cpp)
target_include_directories(dnf2b_src PUBLIC ./ ${SYSTEMD_INCLUDE_DIRS})

# TODO: conditionally link fmt (probably)
# Source library linking
target_link_libraries(dnf2b_src PUBLIC
    stc
    fmt::fmt
    nlohmann_json
    spdlog
    cpr::cpr
    SQLiteCpp
    sqlite3
    pthread
    dl
    pcre2-posix-static

    asio::asio
    CLI11

    ${SYSTEMD_LIBRARIES}
)

# Executable linking
target_link_libraries(dnf2b PUBLIC dnf2b_src)

# Installation {{{
install(DIRECTORY ../etc/dnf2b ../etc/systemd DESTINATION /etc)
install(TARGETS dnf2b
    DESTINATION bin)
install(
    DIRECTORY ${CMAKE_BINARY_DIR}/lib
    DESTINATION lib
    FILES_MATCHING
        PATTERN *.a EXCLUDE
)

#install(TARGETS dnf2b)
#install(IMPORTED_RUNTIME_ARTIFACTS cpr::cpr fmt::fmt spdlog::spdlog SQLiteCpp)
#install(DIRECTORY ${CMAKE_BINARY_DIR}/lib DESTINATION .)
# }}}

# vim:ft=cmake
