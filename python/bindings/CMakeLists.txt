include_directories(${Python3_INCLUDE_DIRS})
get_target_property(PYTHON_COMMON_SOURCE_DIR pyariadne-python-common SOURCE_DIR)

add_library(pyariadne-system-submodule-obj OBJECT system_submodule.cpp)
target_include_directories(pyariadne-system-submodule-obj PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${PYTHON_COMMON_SOURCE_DIR}/include"
    $<TARGET_PROPERTY:pyariadne-hybrid,INTERFACE_INCLUDE_DIRECTORIES>
)
target_link_libraries(pyariadne-system-submodule-obj PRIVATE
    pyariadne-python-common
    pybind11::module
    system
)
set_target_properties(pyariadne-system-submodule-obj PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(pyariadne-system INTERFACE)
target_sources(pyariadne-system INTERFACE
    $<TARGET_OBJECTS:pyariadne-system-submodule-obj>
)
target_include_directories(pyariadne-system INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<TARGET_PROPERTY:pyariadne-hybrid,INTERFACE_INCLUDE_DIRECTORIES>
)
target_link_libraries(pyariadne-system INTERFACE pyariadne-hybrid)

ariadne_add_python_module(NAME system MODULE_SOURCE system_module.cpp)
