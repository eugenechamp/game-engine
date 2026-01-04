add_executable(Sandbox)

option(BUILD_SHARED_LIBS OFF)

target_sources(Sandbox PRIVATE src/main.cpp)

target_link_libraries(Sandbox PUBLIC engine-runtime)
message("Cmake source dir is " ${CMAKE_SOURCE_DIR})

# message("Cmake source dir is " ${CMAKE_CURRENT})

set_target_properties(Sandbox PROPERTIES 
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>")

if(MSVC)
  set_target_properties(Sandbox PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY  "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>")
else()
  set_target_properties(Sandbox PROPERTIES DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>")
endif()

target_precompile_headers(Sandbox
PUBLIC
  <iostream>
  <optional>
  <filesystem>
  <vector>
  <chrono>
  <utility>
  <cstdint>
  <memory>
  <span>
  <set>
  <source_location>
  <fmt/core.h>
  <unordered_map>
  <map>
  <functional>
  <cstring>
  <string>
  <string_view>
  <type_traits>
  <cstdint>
  <cstddef>
  <glm/glm.hpp>
  <thread>
  <mutex>
  <atomic>
)