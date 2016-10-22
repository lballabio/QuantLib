
include(${CMAKE_CURRENT_LIST_DIR}/quantlib.cmake)

#-------------------------------------------------------------------------------
#  FindBoost functions & macros
#

############################################
#
# Check the existence of the libraries.
#
############################################
# This macro was taken directly from the FindQt4.cmake file that is included
# with the CMake distribution. This is NOT my work. All work was done by the
# original authors of the FindQt4.cmake file. Only minor modifications were
# made to remove references to Qt and make this file more generally applicable
# And ELSE/ENDIF pairs were removed for readability.
#########################################################################

macro(_QuantLib_ADJUST_LIB_VARS basename)
  if(QuantLib_INCLUDE_DIR)
    if(QuantLib_${basename}_LIBRARY_DEBUG AND QuantLib_${basename}_LIBRARY_RELEASE)
      # if the generator supports configuration types then set
      # optimized and debug libraries, or if the CMAKE_BUILD_TYPE has a value
      if(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
        set(QuantLib_${basename}_LIBRARY optimized ${QuantLib_${basename}_LIBRARY_RELEASE} debug ${QuantLib_${basename}_LIBRARY_DEBUG})
      else()
        # if there are no configuration types and CMAKE_BUILD_TYPE has no value
        # then just use the release libraries
        set(QuantLib_${basename}_LIBRARY ${QuantLib_${basename}_LIBRARY_RELEASE} )
      endif()
      # FIXME: This probably should be set for both cases
      set(QuantLib_${basename}_LIBRARIES optimized ${QuantLib_${basename}_LIBRARY_RELEASE} debug ${QuantLib_${basename}_LIBRARY_DEBUG})
    endif()

    # if only the release version was found, set the debug variable also to the release version
    if(QuantLib_${basename}_LIBRARY_RELEASE AND NOT QuantLib_${basename}_LIBRARY_DEBUG)
      set(QuantLib_${basename}_LIBRARY_DEBUG ${QuantLib_${basename}_LIBRARY_RELEASE})
      set(QuantLib_${basename}_LIBRARY       ${QuantLib_${basename}_LIBRARY_RELEASE})
      set(QuantLib_${basename}_LIBRARIES     ${QuantLib_${basename}_LIBRARY_RELEASE})
    endif()

    # if only the debug version was found, set the release variable also to the debug version
    if(QuantLib_${basename}_LIBRARY_DEBUG AND NOT QuantLib_${basename}_LIBRARY_RELEASE)
      set(QuantLib_${basename}_LIBRARY_RELEASE ${QuantLib_${basename}_LIBRARY_DEBUG})
      set(QuantLib_${basename}_LIBRARY         ${QuantLib_${basename}_LIBRARY_DEBUG})
      set(QuantLib_${basename}_LIBRARIES       ${QuantLib_${basename}_LIBRARY_DEBUG})
    endif()

    # If the debug & release library ends up being the same, omit the keywords
    if(${QuantLib_${basename}_LIBRARY_RELEASE} STREQUAL ${QuantLib_${basename}_LIBRARY_DEBUG})
      set(QuantLib_${basename}_LIBRARY   ${QuantLib_${basename}_LIBRARY_RELEASE} )
      set(QuantLib_${basename}_LIBRARIES ${QuantLib_${basename}_LIBRARY_RELEASE} )
    endif()

    if(QuantLib_${basename}_LIBRARY)
      set(QuantLib_${basename}_FOUND ON)
    endif()

  endif()
  # Make variables changeable to the advanced user
  mark_as_advanced(
      QuantLib_${basename}_LIBRARY_RELEASE
      QuantLib_${basename}_LIBRARY_DEBUG
  )
endmacro()

# Here I could iterate through components (but actually we have only one)
get_quantlib_library_name(QL_LIBRARY_NAME)
message("** Looking for library ${QL_LIBRARY_NAME}")

find_path(QuantLib_INCLUDE_DIR NAMES ql/quantlib.hpp PATHS ${QUANTLIB_ROOT})
find_library(QuantLib_Unique_LIBRARY_RELEASE NAMES ${QL_LIBRARY_NAME} PATHS ${CONAN_LIB_DIRS_QUANTLIB})
find_library(QuantLib_Unique_LIBRARY_DEBUG NAMES ${QL_LIBRARY_NAME}${CMAKE_DEBUG_POSTFIX} PATHS ${CONAN_LIB_DIRS_QUANTLIB})
_QuantLib_ADJUST_LIB_VARS(Unique)

if(QuantLib_Unique_FOUND)
    set(QuantLib_FOUND 1)
    set(QuantLib_INCLUDE_DIRS ${QuantLib_INCLUDE_DIR})
    set(QuantLib_LIBRARIES ${QuantLib_Unique_LIBRARY})

    message("** QUANTLIB found!")
    message("   - libraries:  ${QuantLib_LIBRARIES}")
    message("   - includes:  ${QuantLib_INCLUDE_DIRS}")
else()
    message("** QUANTLIB not found!")
    set(QuantLib_FOUND 0)
endif()

