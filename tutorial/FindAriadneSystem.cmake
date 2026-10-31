find_library(ARIADNE_SYSTEM_LIBRARY NAMES ariadne-system)
find_path(ARIADNE_SYSTEM_INCLUDE_DIR ariadne-system.hpp
          PATH_SUFFIXES ariadne-system)

find_package(Threads REQUIRED)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AriadneSystem DEFAULT_MSG
    ARIADNE_SYSTEM_LIBRARY
    ARIADNE_SYSTEM_INCLUDE_DIR
)

if(AriadneSystem_FOUND)
    get_filename_component(ARIADNE_SYSTEM_INCLUDE_PARENT_DIR
        "${ARIADNE_SYSTEM_INCLUDE_DIR}" DIRECTORY)

    if(NOT TARGET AriadneSystem::ariadne-system)
        add_library(AriadneSystem::ariadne-system UNKNOWN IMPORTED)
        set_target_properties(AriadneSystem::ariadne-system PROPERTIES
            IMPORTED_LOCATION "${ARIADNE_SYSTEM_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES
                "${ARIADNE_SYSTEM_INCLUDE_PARENT_DIR};${ARIADNE_SYSTEM_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES "Threads::Threads"
        )
    endif()
endif()
