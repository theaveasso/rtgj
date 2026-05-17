cmake_minimum_required(VERSION 3.20)

project(RTGJ)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(dependencies)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
	src/platform_win32.cpp
	src/main.cpp 
)

target_link_libraries(${PROJECT_NAME} PRIVATE volk)
target_compile_definitions(${PROJECT_NAME} PRIVATE VK_NO_PROTOTYPES WIN32_LEAN_AND_MEAN)