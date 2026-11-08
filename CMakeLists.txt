cmake_minimum_required(VERSION 3.15)

project(System VERSION 1.0)

option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)

if(NOT TARGET ariadne-build)
    find_package(Git REQUIRED)
    # Keep the initial Build checkout output in English.
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E env "LC_ALL=C"
                "${GIT_EXECUTABLE}" -C "${CMAKE_CURRENT_SOURCE_DIR}"
                submodule update --init -- ss/bui
        RESULT_VARIABLE _result)
    if(_result)
        message(FATAL_ERROR "Cannot initialize build submodule")
    endif()
    include("${CMAKE_CURRENT_SOURCE_DIR}/ss/bui/cmake/ProjectOptions.cmake")
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # Pinned child commits guarantee internal consistency.
    require_same_dependency_commit(Build CHAINS
        "build" "hybrid|build" "neural-set|build")
    require_same_dependency_commit(Set CHAINS
        "hybrid|labelled|expression-set|set" "neural-set|set")
endif()

setup_project()
include_directories("${PROJECT_SOURCE_DIR}/include")
add_subdirectory(ss)

add_library(system INTERFACE)
target_include_directories(system INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/ariadne-system>
)
target_link_libraries(system INTERFACE hybrid neural_set)

add_subdirectory(src)
target_link_libraries(SYSTEM_SRC PRIVATE system)

ariadne_register_public_headers(
    TARGET system
    DIRECTORIES "${PROJECT_SOURCE_DIR}/include/system"
    FILES "${PROJECT_SOURCE_DIR}/include/ariadne-system.hpp"
)

setup_project_library(
    TARGET ariadne-system
    OBJECTS SYSTEM_SRC
    LINK_LIBRARIES system
)

setup_standalone_project(
    TARGET ariadne-system
    TEST_DIRECTORY tests
    TEST_TARGET tests
    SOURCES
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/src"
)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    ariadne_install_dependency_bundle(
        TARGET system
        DESTINATION include/ariadne-system
    )
endif()

if(BUILD_PYTHON_BINDINGS)
    add_subdirectory(python)
endif()
