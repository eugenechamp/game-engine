cmake_minimum_required(VERSION 3.14..3.24)

option(BUILD_SHARED_LIBS OFF)

project(Engine 
  VERSION 0.0.0
  DESCRIPTION "The first generation of my game engine"
  LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20) # not the best practice, but who cares

if(MSVC)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
  set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")
endif()

add_subdirectory(engine)
add_subdirectory(3rd_party)
add_subdirectory(sandbox)
#TODO: fix that abomination
add_subdirectory(build/_deps/jolt-src/Build)

set_target_properties(Jolt PROPERTIES 
  FOLDER "Deps"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>"
)

if(MSVC)
  target_compile_options(Jolt PRIVATE /MP /W4 /GR-)
else()
  target_compile_options(Jolt -Wall -fno-rtti -fno-exceptions -Wextra -Wpedantic)
endif()

if(MSVC) 
  add_compile_options(/MP /W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -fno-exceptions)
endif()