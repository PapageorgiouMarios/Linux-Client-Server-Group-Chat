cmake_minimum_required(VERSION 3.10)
project(CppLinuxChat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directory (so "client.h" and "server.h" are found)
include_directories(${CMAKE_SOURCE_DIR})

# ===== Server Tests =====
add_executable(test_server
    test_server.cpp
    server.cpp
)

target_link_libraries(test_server pthread)
# ========================


# ===== Client Tests =====
add_executable(test_client
    test_client.cpp
    client.cpp
    server.cpp
)

target_link_libraries(test_client pthread)
# ========================

# ===== Main Server executables =====
add_executable(main_server
    main_server.cpp
    server.cpp
)
target_link_libraries(main_server pthread)
# ===================================

# ===== Main Client executable =====
add_executable(main_client
    main_client.cpp
    client.cpp
)
target_link_libraries(main_client pthread)
# ===================================