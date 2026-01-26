cmake_minimum_required(VERSION 3.10)

project(endlessh-report VERSION 1.3.0 LANGUAGES CXX DESCRIPTION "A simple report generator for endlessh tarpits.")
set(PROJECT_LONG_NAME "Endlessh Report Generator")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(ENABLE_COVERAGE "Enable compilation flags for gcov coverage" ON)

include(CTest)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/include/version.hpp)

add_compile_options(
    -Wpedantic
    -Werror
    -Wall
    -Wno-unused-function
)

include_directories(
    include/
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

if (DEFINED endlesshreport_DEBUG OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(
        -O0
        -ggdb
    )
else()
    add_compile_options(
        -O2
    )
endif()

if (ENABLE_COVERAGE)
    message(STATUS "Building with coverage support (--coverage)")
    add_compile_options(--coverage -O0 -g)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
endif()

###
# Make sure submodules are updated
###
message("Ensuring repository has all submodules...")
execute_process(
    COMMAND git submodule update --init --recursive
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/submodules/date)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/submodules/fmt)

add_library(endlessh_core STATIC src/core.cpp)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        endlessh_core
        date
        fmt
)

target_link_libraries(endlessh_core PRIVATE fmt date)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()

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

# Coverage target that runs the coverage helper script
add_custom_target(coverage
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/coverage.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Run tests and generate coverage report (requires lcov/genhtml)"
)
