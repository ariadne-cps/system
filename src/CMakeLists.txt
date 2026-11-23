add_library(SYSTEM_SRC OBJECT
    system_function.cpp
    ordinary_dynamics.cpp
    odae.cpp
    iterated_map.cpp
    difference_algebraic_equation.cpp
    discrete_time_hybrid_automaton.cpp
    odai.cpp
    first_order_pde.cpp
    odai_grid.cpp
    continuous_time_hybrid_automaton.cpp
    composite_continuous_time_hybrid_automaton.cpp
    hybrid_paving.cpp
)
target_include_directories(SYSTEM_SRC PRIVATE "${PROJECT_SOURCE_DIR}/include/system")
