cmake_minimum_required(VERSION 3.29)
project(WordExchange)

set(CMAKE_CXX_STANDARD 23)
include_directories(sqlite hash256)
add_compile_definitions(SQLITE_THREADSAFE=1)

add_executable(WordExchange sqlite/sqlite3.h sqlite/sqlite3.c
        src/User.h
        src/User.cpp
        src/UserDAO.cpp
        src/UserDAO.h
        src/DatabaseConnection.cpp
        src/DatabaseConnection.h
        hash256/sha256.cpp
        hash256/sha256.h
        hash256/endian.h
        src/Server.cpp
        src/Server.h
        src/userService.cpp
        src/userService.h
        src/patterns.h
        src/PasswordHasher.cpp
        src/PasswordHasher.h
        src/menuService.cpp
        src/menuService.h
        src/GameDAO.cpp
        src/GameDAO.h
        src/Session.cpp
        src/Session.h
)

target_link_libraries(WordExchange ws2_32)

