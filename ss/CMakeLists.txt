if(NOT TARGET hybrid)
    ariadne_add_submodule(hybrid)
endif()

# Testing is for test targets only, not for production libraries.
if(NOT TARGET ariadne-testing)
    ariadne_add_submodule(testing)
endif()
