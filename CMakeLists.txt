cmake_minimum_required(VERSION 3.0.0)
project(some VERSION 0.1.0)

include(CTest)
enable_testing()

if(UNIX)
link_libraries(pthread)
endif()
include_directories(PUBLIC 
    ${CMAKE_SOURCE_DIR}/some 
    ${CMAKE_SOURCE_DIR}/Thirdparty)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
add_executable(positional Examples/some_positional.cpp)
add_test( NAME positional COMMAND positional )
add_executable(indexable Examples/some.cpp)
add_test( NAME Indexable  COMMAND indexable )
add_executable(pbar Examples/pbar.cpp)
add_test( NAME pbar  COMMAND pbar )


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
