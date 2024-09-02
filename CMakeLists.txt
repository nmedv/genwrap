cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(genwrap)

add_subdirectory("utl")

add_executable(genwrap
	src/main.cpp
	src/genwrap.cpp
	src/windows.cpp
)
target_include_directories(genwrap PRIVATE public)
target_link_libraries(genwrap utl Shlwapi)

add_library(wrapper STATIC src/wrapper/wrapper.cpp)

target_include_directories(wrapper PRIVATE public)