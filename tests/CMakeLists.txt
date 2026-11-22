include(CTest)
set(UNIT_TESTS
    test_discrete_event
    test_iterated_map
    test_vector_field
    test_hybrid_automaton
    test_hybrid_automaton_api
    test_hybrid_automaton_composite
    test_differential_inclusion
    test_first_order_pde
    test_first_order_pde_model
    test_first_order_pde_model_header
    test_system_header
)
foreach(TEST IN LISTS UNIT_TESTS)
    add_executable(${TEST} ${TEST}.cpp)
    target_link_libraries(${TEST} PRIVATE ariadne-system system ariadne-testing)
    add_test(NAME ${TEST} COMMAND $<TARGET_FILE:${TEST}> --concise --logtonull)
    set_property(TEST ${TEST} PROPERTY LABELS "system")
endforeach()
add_custom_target(tests)
add_dependencies(tests ${UNIT_TESTS})
