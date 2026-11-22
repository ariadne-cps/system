add_library(SYSTEM_SRC OBJECT
    system_function.cpp
    vector_field.cpp
    iterated_map.cpp
    differential_inclusion.cpp
    first_order_pde.cpp
    hybrid_automaton.cpp
    hybrid_automaton-composite.cpp
)
target_include_directories(SYSTEM_SRC PRIVATE "${PROJECT_SOURCE_DIR}/include/system")
