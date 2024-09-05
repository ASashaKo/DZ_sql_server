cmake_minimum_required(VERSION 3.5)

project(chat_client VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
    main.cpp
    sha1.h
    sha1.cpp
    Message.h
    Message.cpp
    TCPClient.h
    TCPClient.cpp
)

include_directories(include
    -I$(/usr/include/mysql/)
    -L$(/usr/lib/x86_64-linux-gnu)
)

add_executable(chat_client
            ${PROJECT_SOURCES}
        )
        
target_link_libraries(chat_client PRIVATE
    -lmysqlclient
)

install(TARGETS chat_client
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
