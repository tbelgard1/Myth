cmake_minimum_required(VERSION 3.10)
project(BungieNetServer)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add Windows-specific definitions
if(WIN32)
    add_definitions(-D_WINSOCKAPI_)
    add_definitions(-D_WINSOCK2API_)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
    add_definitions(-D_USE_WINSOCK2)
    add_definitions(-D_WIN32_LEAN_AND_MEAN)
    add_definitions(-D_WIN32_WINNT=0x0601)
    
    # Link against ws2_32.lib
    link_libraries(ws2_32)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/game_search_new
    ${CMAKE_CURRENT_SOURCE_DIR}/room_new
    ${CMAKE_CURRENT_SOURCE_DIR}/users_new
)

# Add source files
file(GLOB_RECURSE SOURCES
    "common/*.c"
    "game_search_new/*.c"
    "room_new/*.c"
    "users_new/*.c"
)

# Create executable
add_executable(BungieNetServer ${SOURCES})
