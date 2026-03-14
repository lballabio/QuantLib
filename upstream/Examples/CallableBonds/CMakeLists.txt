add_executable(CallableBonds CallableBonds.cpp)
target_link_libraries(CallableBonds ql_library ${QL_THREAD_LIBRARIES})
if (QL_INSTALL_EXAMPLES)
    install(TARGETS CallableBonds RUNTIME DESTINATION ${QL_INSTALL_EXAMPLESDIR})
endif()

