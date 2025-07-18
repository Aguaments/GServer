cmake_minimum_required(VERSION 3.0)

project(Agent VERSION 1.0)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-funtcion")

set(LIB_SRC 
    src/log.cpp
    src/utils.cpp
    )

include_directories(src)

add_library(log SHARED ${LIB_SRC})

add_executable(AgentServer test/test_log.cpp)
target_link_libraries(AgentServer log)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)