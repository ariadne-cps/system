include(CTest)
set(UNIT_TESTS
    test_discrete_event
    test_hybrid_automaton
    test_differential_inclusion_grid
    test_first_order_pde_model_header
    test_system_header
)
foreach(TEST IN LISTS UNIT_TESTS)
    add_executable(${TEST} ${TEST}.cpp)
    target_link_libraries(${TEST} ariadne-system system)
    add_test(NAME ${TEST} COMMAND $<TARGET_FILE:${TEST}>)
    set_property(TEST ${TEST} PROPERTY LABELS "system")
endforeach()
add_custom_target(tests)
add_dependencies(tests ${UNIT_TESTS})
