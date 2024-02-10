cmake_minimum_required(VERSION 3.10)
project(gitrepublic-ndk)

# Build the project.
set(NOSTR_INCLUDE_DIR ../../include/nostr)
include_directories(${NOSTR_INCLUDE_DIR})
set(HEADERS ${NOSTR_INCLUDE_DIR}/nostr.hpp)

set(SOURCES
    event.cpp
    nostr_utils.cpp
)

find_package(Boost REQUIRED COMPONENTS
    random
    system
)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(plog CONFIG REQUIRED)
find_package(websocketpp CONFIG REQUIRED)

add_library(gitrepublic-ndk ${SOURCES} ${HEADERS})

target_link_libraries(gitrepublic-ndk PRIVATE
    Boost::random
    Boost::system
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
    plog::plog
    websocketpp::websocketpp
)

# Build the tests.
set(TEST_DIR ../../test/nostr)
set(TEST_SOURCES
    ${TEST_DIR}/nostr_utils_test.cc
)

find_package(GTest CONFIG REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(
    gitrepublic-ndk-test
    ${TEST_SOURCES}
    ${HEADERS}
)

target_link_libraries(gitrepublic-ndk-test PRIVATE
    GTest::gtest
    GTest::gtest_main
    gitrepublic-ndk
)
gtest_add_tests(TARGET gitrepublic-ndk-test)
