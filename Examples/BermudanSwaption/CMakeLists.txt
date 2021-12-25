add_executable(BermudanSwaption BermudanSwaption.cpp)
target_link_libraries(BermudanSwaption ql_library ${QL_EXAMPLES_LIBRARIES})
if (QL_INSTALL_EXAMPLES)
    install(TARGETS BermudanSwaption RUNTIME DESTINATION ${QL_INSTALL_EXAMPLESDIR})
endif()

