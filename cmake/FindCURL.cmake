include(FindPackageHandleStandardArgs)

if(WIN32)
    set(CURL_ROOT_DIR "C:/curl" CACHE PATH "CURL root directory")

    find_path(CURL_INCLUDE_DIR
        NAMES curl/curl.h
        PATHS
            ${CURL_ROOT_DIR}/include
            $ENV{CURL_ROOT}/include
            "C:/Program Files/curl/include"
            "C:/curl/include"
    )

    find_library(CURL_LIBRARY
        NAMES libcurl libcurl_imp curl
        PATHS
            ${CURL_ROOT_DIR}/lib
            $ENV{CURL_ROOT}/lib
            "C:/Program Files/curl/lib"
            "C:/curl/lib"
    )
elseif(APPLE)
    find_path(CURL_INCLUDE_DIR
        NAMES curl/curl.h
        PATHS
            /usr/local/include
            /opt/homebrew/include
            /opt/local/include
    )
    
    find_library(CURL_LIBRARY
        NAMES curl
        PATHS
            /usr/local/lib
            /opt/homebrew/lib
            /opt/local/lib
    )
else()
    find_path(CURL_INCLUDE_DIR
        NAMES curl/curl.h
        PATHS
            /usr/include
            /usr/local/include
    )
    
    find_library(CURL_LIBRARY
        NAMES curl
        PATHS
            /usr/lib
            /usr/local/lib
            /usr/lib/x86_64-linux-gnu
    )
endif()

find_package_handle_standard_args(CURL
    REQUIRED_VARS CURL_LIBRARY CURL_INCLUDE_DIR
)

if(CURL_FOUND)
    set(CURL_INCLUDE_DIRS ${CURL_INCLUDE_DIR})
    set(CURL_LIBRARIES ${CURL_LIBRARY})
    
    if(NOT TARGET CURL::libcurl)
        add_library(CURL::libcurl UNKNOWN IMPORTED)
        set_target_properties(CURL::libcurl PROPERTIES
            IMPORTED_LOCATION "${CURL_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIR}"
        )
    endif()
endif()

mark_as_advanced(CURL_INCLUDE_DIR CURL_LIBRARY)
