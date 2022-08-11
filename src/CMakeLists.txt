set (SOURCE_FILES
    dnf2b/core/Context.cpp

    dnf2b/ui/CLI.cpp
)

add_library(dnf2b_src STATIC ${SOURCE_FILES})

add_executable(doesnotfail2ban dnf2b/Main.cpp)
target_include_directories(dnf2b_src PUBLIC ./)
#target_include_directories(doesnotfail2ban PUBLIC ${CONAN_INCLUDE_DIRS})

target_link_libraries(dnf2b_src PUBLIC nlohmann_json)

#target_link_libraries(doesnotfail2ban ${CONAN_LIBS})
target_link_libraries(doesnotfail2ban PUBLIC dnf2b_src)

# vim:ft=cmake
