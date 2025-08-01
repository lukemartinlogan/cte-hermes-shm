project(hermes_shm)

# ------------------------------------------------------------------------------
# Build Tests
# ------------------------------------------------------------------------------
add_executable(test_solvers_exec
        ${TEST_MAIN}/main.cc
        test_init.cc
        test_nonlinear_least_squares.cc)
add_dependencies(test_solvers_exec hermes_shm_host)
target_link_libraries(test_solvers_exec
        hermes_shm_host Catch2::Catch2)

# ------------------------------------------------------------------------------
# Test Cases
# ------------------------------------------------------------------------------

# Nonlinear Least Squares tests
add_test(NAME test_NonlinearLeastSquares_QuadraticFitting COMMAND
        ${CMAKE_BINARY_DIR}/bin/test_solvers_exec "NonlinearLeastSquares_QuadraticFitting")

add_test(NAME test_NonlinearLeastSquares_ExponentialFitting COMMAND
        ${CMAKE_BINARY_DIR}/bin/test_solvers_exec "NonlinearLeastSquares_ExponentialFitting")

add_test(NAME test_NonlinearLeastSquares_EmptyParameters COMMAND
        ${CMAKE_BINARY_DIR}/bin/test_solvers_exec "NonlinearLeastSquares_EmptyParameters")

add_test(NAME test_NonlinearLeastSquares_ParameterAccess COMMAND
        ${CMAKE_BINARY_DIR}/bin/test_solvers_exec "NonlinearLeastSquares_ParameterAccess")

add_test(NAME test_NonlinearLeastSquares_LinearCase COMMAND
        ${CMAKE_BINARY_DIR}/bin/test_solvers_exec "NonlinearLeastSquares_LinearCase")

# ------------------------------------------------------------------------------
# Install Targets
# ------------------------------------------------------------------------------
install(TARGETS
        test_solvers_exec
        LIBRARY DESTINATION ${HSHM_INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${HSHM_INSTALL_LIB_DIR}
        RUNTIME DESTINATION ${HSHM_INSTALL_BIN_DIR})

# -----------------------------------------------------------------------------
# Coverage
# -----------------------------------------------------------------------------
if(HSHM_ENABLE_COVERAGE)
        set_coverage_flags(test_solvers_exec)
endif()