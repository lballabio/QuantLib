add_executable(Bonds Bonds.cpp)
target_link_libraries(Bonds ql_library ${QL_EXAMPLES_LIBRARIES})
if (QL_INSTALL_EXAMPLES)
    install(TARGETS Bonds RUNTIME DESTINATION ${QL_INSTALL_EXAMPLESDIR})
endif()

