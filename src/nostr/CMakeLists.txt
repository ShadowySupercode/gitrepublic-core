cmake_minimum_required(VERSION 3.14)
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
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(TEST_DIR ../../test)
set(TEST_SOURCES
    ${TEST_DIR}/nostr/nostr_service_test.cpp
)

add_executable(NostrSDKTest ${TEST_SOURCES} ${HEADERS})
target_link_libraries(NostrSDKTest PRIVATE
    GTest::gmock
    GTest::gtest
    GTest::gtest_main
    NostrSDK
    plog::plog
    websocketpp::websocketpp
)
set_target_properties(NostrSDKTest PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS YES)

include(GoogleTest)
gtest_add_tests(TARGET NostrSDKTest)
