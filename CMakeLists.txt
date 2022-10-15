cmake_minimum_required(VERSION 3.10)

project(endlessh-report VERSION 1.2.1 LANGUAGES CXX DESCRIPTION "A simple report generator for endlessh tarpits.")
set(PROJECT_LONG_NAME "Endlessh Report Generator")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/include/version.hpp)

add_compile_options(
    -Wpedantic
    -Werror
    -Wall
)

include_directories(
    include/
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

if (DEFINED endlesshreport_DEBUG)
    add_compile_options(
        -O0
        -ggdb
    )
else()
    add_compile_options(
        -O2
        -s
    )
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/submodules/date)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/submodules/fmt)

file(GLOB_RECURSE FILES FOLLOW_SYMLINKS ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)

add_executable(
    ${PROJECT_NAME}

    ${FILES}
)

target_link_libraries(
    ${PROJECT_NAME}

    date
    fmt
)

###
# Docs target
###
add_custom_target("docs" COMMENT "Create Doxygen documentation")
add_custom_command(
    TARGET "docs"
    POST_BUILD
        COMMENT "Generate Doxygen documentation for publication or reading"
        COMMAND doxygen ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

###
# Install and Remove targets and their configs go here
###
set(endlesshreport_INSTALL_DIR "/usr/local/bin")

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${endlesshreport_INSTALL_DIR}
)

add_custom_target("uninstall" COMMENT "Uninstall endlesshreport")
add_custom_command(
    TARGET "uninstall"
    POST_BUILD
        COMMENT "Uninstall endlesshreport from the current system"
        COMMAND xargs rm -Rfv < install_manifest.txt || echo "Nothing to be removed from system!"
)

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_CONTACT "Simon Cahill <simon@simonc.eu>")
set(CPACK_PACKAGE_VENDOR "Simon Cahill")
set(CPACK_GENERATOR DEB)
include(CPack)
