cmake_minimum_required(VERSION 3.5)
project(gitrepublic-ndk)

set(NOSTR_INCLUDE_DIR ../../include/nostr)
include_directories(${NOSTR_INCLUDE_DIR})
set(HEADERS ${NOSTR_INCLUDE_DIR}/nostr.hpp)

set(SOURCES
    event.cpp
    nostr_utils.cpp
)

find_package(Boost REQUIRED COMPONENTS system)
find_package(websocketpp CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(plog CONFIG REQUIRED)

add_library(gitrepublic-ndk ${SOURCES} ${HEADERS})

target_link_libraries(gitrepublic-ndk PRIVATE Boost::system)
target_link_libraries(gitrepublic-ndk PRIVATE websocketpp::websocketpp)
target_link_libraries(gitrepublic-ndk PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(gitrepublic-ndk PRIVATE plog::plog)
