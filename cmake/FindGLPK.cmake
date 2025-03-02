include(FindPackageHandleStandardArgs)

if(WIN32)
    set(GLPK_ROOT_DIR "C:/glpk-4.65" CACHE PATH "GLPK root directory")

find_path(GLPK_INCLUDE_DIR
    NAMES glpk.h
    PATHS
        ${GLPK_ROOT_DIR}/src
        ${GLPK_ROOT_DIR}/src/glpk
        ${GLPK_ROOT_DIR}/src/api
        $ENV{GLPK_ROOT}/src
)

find_library(GLPK_LIBRARY
    NAMES glpk glpk_4_65 glpk-0
    PATHS
        ${GLPK_ROOT_DIR}/w64
        $ENV{GLPK_ROOT}/w64
)
elseif(APPLE)
    find_path(GLPK_INCLUDE_DIR
        NAMES glpk.h
        PATHS
            /usr/local/include
            /opt/homebrew/include
            /opt/local/include
    )
    
    find_library(GLPK_LIBRARY
        NAMES glpk
        PATHS
            /usr/local/lib
            /opt/homebrew/lib
            /opt/local/lib
    )
else()
    find_path(GLPK_INCLUDE_DIR
        NAMES glpk.h
        PATHS
            /usr/include
            /usr/local/include
    )
    
    find_library(GLPK_LIBRARY
        NAMES glpk
        PATHS
            /usr/lib
            /usr/local/lib
            /usr/lib/x86_64-linux-gnu
    )
endif()

find_package_handle_standard_args(GLPK
    REQUIRED_VARS GLPK_LIBRARY GLPK_INCLUDE_DIR
)

if(GLPK_FOUND)
    set(GLPK_INCLUDE_DIRS ${GLPK_INCLUDE_DIR})
    set(GLPK_LIBRARIES ${GLPK_LIBRARY})
    
    if(NOT TARGET GLPK::GLPK)
        add_library(GLPK::GLPK UNKNOWN IMPORTED)
        set_target_properties(GLPK::GLPK PROPERTIES
            IMPORTED_LOCATION "${GLPK_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${GLPK_INCLUDE_DIR}"
        )
    endif()
endif()

mark_as_advanced(GLPK_INCLUDE_DIR GLPK_LIBRARY)
