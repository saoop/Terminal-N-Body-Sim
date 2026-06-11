find_package(OpenMP REQUIRED)
target_link_libraries(sim PUBLIC OpenMP::OpenMP_CXX)