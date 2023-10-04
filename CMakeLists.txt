
cmake_minimum_required(VERSION 3.16)
project(dcf)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wall -fmessage-length=0 -m64 -static-libgcc -Wl,--no-as-needed")
set(CMAKE_INSTALL_RPATH "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_CXX_COMPILER_FLAGS "/usr/bin/g++")
set(CMAKE_CXX_STANDARD 20)

set(SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/manager.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/receiver.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/sender.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/medium.cpp
)

set(HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/receiver.hpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/include/manager.hpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/include/receiver.hpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/include/sender.hpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/include/medium.hpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
                           PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}/include
)