cmake_minimum_required(VERSION 3.15)
project(AriadneSystemTutorial LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compile against an installed Ariadne System package.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
find_package(AriadneSystem REQUIRED)

add_executable(tutorial_system tutorial_system.cpp)
target_link_libraries(tutorial_system PRIVATE AriadneSystem::ariadne-system)
