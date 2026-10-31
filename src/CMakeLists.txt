add_library(SYSTEM_SRC OBJECT
    system_function.cpp
    vector_field.cpp
    iterated_map.cpp
    differential_inclusion.cpp
    differential_inclusion_grid.cpp
    hybrid_automaton.cpp
    hybrid_automaton-composite.cpp
    hybrid_paving.cpp
)
target_include_directories(SYSTEM_SRC PRIVATE "${PROJECT_SOURCE_DIR}/include/system")
