option(ENABLE_SESSIONS "Enables sessions" OFF)

if (ENABLE_SESSIONS)
    find_package(Boost REQUIRED COMPONENTS thread)
    include_directories(${Boost_INCLUDE_DIRS})
    list(APPEND EXTRA_LIBS ${Boost_LIBRARIES})
    add_definitions(-DQL_ENABLE_SESSIONS)
endif()
