if(NOT TARGET hybrid)
    ariadne_add_submodule(hybrid)
endif()
if(NOT TARGET neural_set)
    ariadne_add_submodule(neural-set)
endif()

# Testing is for test targets only, not for production libraries.
if(NOT TARGET ariadne-testing)
    ariadne_add_submodule(testing)
endif()
