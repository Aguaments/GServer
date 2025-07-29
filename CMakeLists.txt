cmake_minimum_required(VERSION 3.28.3)

project(Agent VERSION 1.0)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-funtcion")

include_directories(src)

# set(LOG_LIB_SRC src/log.cpp)
# set(UTILS_LIB_SRC src/utils.cpp)
# set(CONFIG_LIB_SRC src/config.cpp)
# set(THREAD_LIB_SRC src/thread.cpp)


# add_library(log SHARED ${LOG_LIB_SRC})
# add_library(utils SHARED ${UTILS_LIB_SRC})
# add_library(config SHARED ${CONFIG_LIB_SRC})
# add_library(thread SHARED ${THREAD_LIB_SRC})

# add_executable(test_log test/test_log.cpp)
# target_link_libraries(test_log
#     log
#     yaml-cpp
#     utils)

set(AGENT_LIB_SRC
    src/utils.cpp
    src/log.cpp
    src/config.cpp
    src/thread.cpp
    src/coroutine.cpp
    src/scheduler.cpp)

add_library(agent SHARED ${AGENT_LIB_SRC})

set(LIB_LIB
    agent
    pthread
    yaml-cpp)

# add_executable(test_thread test/test_thread.cpp)
# target_link_libraries(test_thread ${LIB_LIB})

# add_executable(test_config test/test_config.cpp)
# target_link_libraries(test_config ${LIB_LIB})

# add_executable(test_util test/test_util.cpp)
# target_link_libraries(test_util ${LIB_LIB})

add_executable(test_coroutine test/test_coroutine.cpp)
target_link_libraries(test_coroutine ${LIB_LIB})

add_executable(test_scheduler test/test_scheduler.cpp)
target_link_libraries(test_scheduler ${LIB_LIB})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)