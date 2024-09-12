cmake_minimum_required(VERSION 3.29)
project(WordExchange)

set(CMAKE_CXX_STANDARD 23)
include_directories(sqlite hash256)
add_compile_definitions(SQLITE_THREADSAFE=1)

add_executable(WordExchange sqlite/sqlite3.h sqlite/sqlite3.c
        src/Objects/User.h
        src/Objects/User.cpp
        src/DAOs/UserDAO.cpp
        src/DAOs/UserDAO.h
        src/DAOs/DatabaseConnection.cpp
        src/DAOs/DatabaseConnection.h
        hash256/sha256.cpp
        hash256/sha256.h
        hash256/endian.h
        src/Server.cpp
        src/Server.h
        src/Services/UserService.cpp
        src/Services/UserService.h
        src/Services/Patterns.h
        src/Services/PasswordHasher.cpp
        src/Services/PasswordHasher.h
        src/Services/MenuService.cpp
        src/Services/MenuService.h
        src/DAOs/GameDAO.cpp
        src/DAOs/GameDAO.h
        src/Objects/Session.cpp
        src/Objects/Session.h
)

target_link_libraries(WordExchange ws2_32)

