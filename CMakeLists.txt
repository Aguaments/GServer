cmake_minimum_required(VERSION 3.28.3)

project(Agent VERSION 1.0)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-funtcion")

set(LOG_LIB_SRC src/log.cpp)
set(UTILS_LIB_SRC src/utils.cpp)
set(CONFIG_LIB_SRC src/config.cpp)


include_directories(src)

add_library(log SHARED ${LOG_LIB_SRC})
add_library(utils SHARED ${UTILS_LIB_SRC})
add_library(config SHARED ${CONFIG_LIB_SRC})

add_executable(test_log test/test_log.cpp)
target_link_libraries(test_log
    log
    utils)

add_executable(test_config test/test_config.cpp)
target_link_libraries(test_config 
    log
    utils
    yaml-cpp
    config)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)