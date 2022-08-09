set (SOURCE_FILES
    dnf2b/sources/Reader.cpp

)

add_library(dnf2b_src STATIC ${SOURCE_FILES})

add_executable(doesnotfail2ban dnf2b/Main.cpp)
target_include_directories(doesnotfail2ban PUBLIC ./)
#target_include_directories(doesnotfail2ban PUBLIC ${CONAN_INCLUDE_DIRS})

#target_link_libraries(doesnotfail2ban ${CONAN_LIBS})
target_link_libraries(doesnotfail2ban dnf2b_src yaml-cpp)

# vim:ft=cmake
