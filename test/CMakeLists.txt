find_package(Catch2 3 REQUIRED)

add_executable(tests test.cpp)

target_link_libraries(tests PRIVATE json Catch2::Catch2WithMain)
