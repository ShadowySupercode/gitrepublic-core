cmake_minimum_required(VERSION 3.5)
project(gitrepublic-core)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../out/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../../out/lib/${CMAKE_BUILD_TYPE})

set(SOURCE_DIR ./src)
set(SOURCES ${SOURCE_DIR}/gitrepublic.cpp)

set(INCLUDE_DIR ./include)
include_directories(${INCLUDE_DIR})
set(HEADERS ${INCLUDE_DIR}/gitrepublic.hpp)

set(TEST_DIR ./test)

add_subdirectory(${SOURCE_DIR}/nostr)

enable_testing()

find_package(GTest CONFIG REQUIRED)

add_executable(gitrepublic-core ${SOURCES} ${HEADERS})
add_executable(gitrepublic-core-test ${SOURCES} ${HEADERS} ${TESTS})

target_include_directories(gitrepublic-core PRIVATE ${INCLUDE_DIR})
target_include_directories(gitrepublic-core PUBLIC ${DEPENDENCIES_INCLUDE_DIR})

target_link_libraries(gitrepublic-core PRIVATE gitrepublic-ndk)
target_link_libraries(gitrepublic-core-test PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(gitrepublic-core-test)
