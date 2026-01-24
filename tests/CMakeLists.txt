cmake_minimum_required(VERSION 3.10)

include(FetchContent)

# Prefer system-provided GTest/GMock if available; fall back to FetchContent.
find_package(GTest CONFIG QUIET)

set(USE_SYSTEM_GTEST OFF)
if (GTest_FOUND AND TARGET GTest::gtest_main AND TARGET GTest::gmock)
    set(USE_SYSTEM_GTEST ON)
endif()

if (NOT USE_SYSTEM_GTEST)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    # Prevent overriding parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)
endif()

if (USE_SYSTEM_GTEST)
    set(GTEST_MAIN_TARGET GTest::gtest_main)
    set(GMOCK_TARGET GTest::gmock)
else()
    set(GTEST_MAIN_TARGET gtest_main)
    set(GMOCK_TARGET gmock)
endif()

add_executable(
    endlessh-report-tests
    report_cache_test.cpp
    end_to_end_cache_test.cpp
)

target_link_libraries(
    endlessh-report-tests
    PRIVATE
        ${GTEST_MAIN_TARGET}
        ${GMOCK_TARGET}
        fmt
)

target_include_directories(
    endlessh-report-tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_BINARY_DIR}/include
)

# Allow end-to-end tests to locate the built binary deterministically.
target_compile_definitions(
    endlessh-report-tests
    PRIVATE
        ENDLESSH_REPORT_BINARY_PATH="$<TARGET_FILE:endlessh-report>"
)

add_dependencies(endlessh-report-tests endlessh-report)

include(GoogleTest)
gtest_discover_tests(endlessh-report-tests)
