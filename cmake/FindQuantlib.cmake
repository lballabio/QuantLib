
include(${CMAKE_CURRENT_LIST_DIR}/quantlib.cmake)
get_quantlib_library_name(QL_LIBRARY_NAME)

find_path(QUANTLIB_INCLUDE_DIR NAMES ql/quantlib.hpp PATHS ${CONAN_INCLUDE_DIRS_QUANTLIB})
find_library(QUANTLIB_LIBRARY NAMES ${QL_LIBRARY_NAME} ${CONAN_LIBS_QUANTLIB} PATHS ${CONAN_LIB_DIRS_QUANTLIB})

if(QUANTLIB_INCLUDE_DIR AND QUANTLIB_LIBRARY)
    set(QUANTLIB_INCLUDE_DIRS ${QUANTLIB_INCLUDE_DIR})
    set(QUANTLIB_LIBRARIES ${QUANTLIB_LIBRARY} ${EXTRA_LIBS})

    mark_as_advanced(QUANTLIB_LIBRARY QUANTLIB_INCLUDE_DIR)

    message("** QUANTLIB found by Conan!")
    set(QUANTLIB_FOUND TRUE)
    message("   - libraries:  ${QUANTLIB_LIBRARIES}")
    message("   - includes:  ${QUANTLIB_INCLUDE_DIRS}")
else()
    message("** QUANTLIB not found as Conan package!")
    set(QUANTLIB_FOUND FALSE)
endif()

