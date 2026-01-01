cmake_minimum_required(VERSION 3.14..3.24)

project(Engine 
  VERSION 0.0.0
  DESCRIPTION "The first generation of my game engine"
  LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20) # not the best practice, but who cares

if(MSVC) 
  add_compile_options(/MP /W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(MSVC)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
  set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")
endif()

add_subdirectory(engine)
add_subdirectory(3rd_party)
add_subdirectory(sandbox)