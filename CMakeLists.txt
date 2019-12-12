cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++17")

option(VERILOGAST_BUILD_TESTS "Build all of verilogAST's own tests." OFF)

project(verilogAST)
set(COVERAGE OFF CACHE BOOL "Coverage")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror")


if (COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g --coverage")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib") 
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

include_directories(
  ${PROJECT_SOURCE_DIR}/include/
) 

set(LIB_SOURCES src/verilogAST.cpp src/transformer.cpp)

set(LIBRARY_NAME verilogAST)
add_library(${LIBRARY_NAME} SHARED ${LIB_SOURCES})

if (VERILOGAST_BUILD_TESTS)
    # Download and unpack googletest at configure time
    configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
      RESULT_VARIABLE result
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
    if(result)
      message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
      RESULT_VARIABLE result
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
    if(result)
      message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif()


    # Add googletest directly to our build. This defines
    # the gtest and gtest_main targets.
    add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                     ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                     EXCLUDE_FROM_ALL)
    enable_testing()

    add_executable(basic tests/basic.cpp)
    target_link_libraries(basic gtest_main ${LIBRARY_NAME})
    add_test(NAME basic_tests COMMAND basic)

    add_executable(parameterized_module tests/parameterized_module.cpp)
    target_link_libraries(parameterized_module gtest_main ${LIBRARY_NAME})
    add_test(NAME parameterized_module_tests COMMAND parameterized_module)

    add_executable(transformer tests/transformer.cpp)
    target_link_libraries(transformer gtest_main ${LIBRARY_NAME})
    add_test(NAME transformer_tests COMMAND transformer)
endif()

install(TARGETS ${LIBRARY_NAME} DESTINATION lib)
install(FILES include/verilogAST.hpp DESTINATION include)
