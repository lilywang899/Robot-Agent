cmake_minimum_required(VERSION 3.12.0)
find_program(CMAKE_C_COMPILER NAMES $ENV{CC} gcc PATHS ENV PATH NO_DEFAULT_PATH)
find_program(CMAKE_CXX_COMPILER NAMES $ENV{CXX} g++ PATHS ENV PATH NO_DEFAULT_PATH)

project(RA)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
 #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs -Werror -fPIC -g -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs  -fPIC -g -O0")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set (LWS_WITH_CLIENT 1)
set (LWS_ROLE_MQTT 1)
set (LWS_WITH_NETWORK 1)

include(FetchContent)
FetchContent_Declare(libwebsockets URL https://github.com/warmcat/libwebsockets/archive/refs/tags/v4.3.3.tar.gz)
FetchContent_MakeAvailable(libwebsockets)

FetchContent_Declare(spdlog URL https://github.com/gabime/spdlog/archive/refs/tags/v1.15.0.tar.gz)
FetchContent_MakeAvailable(spdlog)

add_executable(
        agent
        src/wrapper.cpp
        src/mqttClient.cpp
        src/fdevent.cpp
        src/MessageQueue.cpp
        src/main.cpp
        src/controller.cpp
        src/dsService.cpp
        src/agent.cpp
        src/console.cpp
)
#cmake_print_variables(${LIBWEBSOCKETS_INCLUDE_DIRS})

target_include_directories(
        agent
        PRIVATE
        include
        PUBLIC
        ${LIBWEBSOCKETS_INCLUDE_DIRS}
)

target_link_libraries(
       agent
       #ssl
       #crypto
       websockets
       spdlog
)
#target_compile_options(mqtt PRIVATE -Wall -Wextra -pedantic -Werror -Wno-unused-function)
target_compile_options(agent PRIVATE -Wall -Wextra -Wno-unused-function)