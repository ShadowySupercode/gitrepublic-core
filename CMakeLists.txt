cmake_minimum_required(VERSION 3.5)
project(GitRepublic-Core)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../out/bin/${CMAKE_BUILD_TYPE})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ../../out/lib/${CMAKE_BUILD_TYPE})

set (SOURCE_DIR ./src)

set (SOURCES
    ${SOURCE_DIR}/gitrepublic.cpp
)
set (INCLUDE_DIR ./include)

set (HEADERS
    ${INCLUDE_DIR}/gitrepublic.h
)

add_subdirectory(external)

add_executable(gitrepublic-core ${SOURCES} ${HEADERS})
target_include_directories(gitrepublic-core PRIVATE ${INCLUDE_DIR})
target_link_libraries(gitrepublic-core PRIVATE uWebSockets)
