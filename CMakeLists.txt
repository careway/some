cmake_minimum_required(VERSION 3.0.0)
project(some VERSION 0.1.0)

include(CTest)
enable_testing()


add_library(some some.cpp)
target_include_directories(some PUBLIC Thirdparty)



add_executable(some_test some.cpp )
target_link_libraries(some_test some pthread)
target_include_directories(some_test PUBLIC 
    ${CMAKE_SOURCE_DIR}/some 
    ${CMAKE_SOURCE_DIR}/Thirdparty)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
