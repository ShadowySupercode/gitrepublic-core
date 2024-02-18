cmake_minimum_required(VERSION 3.10)
project(gitrepublic-core)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../out/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../../out/lib/${CMAKE_BUILD_TYPE})

enable_testing()
include(GoogleTest)

set(SOURCE_DIR ./src)
set(SOURCES ${SOURCE_DIR}/gitrepublic.cpp)

set(INCLUDE_DIR ./include)
include_directories(${INCLUDE_DIR})
set(HEADERS ${INCLUDE_DIR}/gitrepublic.hpp)

add_subdirectory(${SOURCE_DIR}/nostr)

add_executable(gitrepublic-core ${SOURCES} ${HEADERS})
add_executable(gitrepublic-core-test ${SOURCES} ${HEADERS} ${TESTS})

target_include_directories(gitrepublic-core PRIVATE ${INCLUDE_DIR})
target_include_directories(gitrepublic-core PUBLIC ${DEPENDENCIES_INCLUDE_DIR})

target_link_libraries(gitrepublic-core PRIVATE NostrSDK)
