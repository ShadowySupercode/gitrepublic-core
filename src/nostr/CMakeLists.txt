cmake_minimum_required(VERSION 3.10)
project(NostrSDK)

# Build the project.
set(NOSTR_INCLUDE_DIR ../../include/nostr)
set(CLIENT_INCLUDE_DIR ../../include/client)
include_directories(${NOSTR_INCLUDE_DIR})
include_directories(${CLIENT_INCLUDE_DIR})
set(HEADERS
    ${NOSTR_INCLUDE_DIR}/nostr.hpp
    ${CLIENT_INCLUDE_DIR}/web_socket_client.hpp
)

set(SOURCES
    event.cpp
    nostr_service.cpp
)

find_package(Boost REQUIRED COMPONENTS random system)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(plog CONFIG REQUIRED)
find_package(websocketpp CONFIG REQUIRED)

add_library(NostrSDK ${SOURCES} ${HEADERS})
target_link_libraries(NostrSDK PRIVATE
    Boost::random
    Boost::system
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
    plog::plog
    websocketpp::websocketpp
)
set_target_properties(NostrSDK PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS YES)

# Build the tests.
set(TEST_DIR ../../test)
set(TEST_SOURCES
    ${TEST_DIR}/nostr/nostr_service_test.cpp
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
find_package(GTest CONFIG REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(NostrSDKTest ${TEST_SOURCES} ${HEADERS})
target_include_directories(NostrSDKTest PRIVATE ../../test/client/include)
target_link_libraries(NostrSDKTest
    GTest::gmock
    GTest::gtest
    GTest::gtest_main
    NostrSDK
    plog::plog
    websocketpp::websocketpp
)
set_target_properties(NostrSDKTest PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS YES)

gtest_add_tests(TARGET NostrSDKTest)
