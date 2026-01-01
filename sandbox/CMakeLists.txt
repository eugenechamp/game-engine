add_executable(Sandbox)

option(BUILD_SHARED_LIBS OFF)

target_sources(Sandbox PRIVATE src/main.cpp)

target_link_libraries(Sandbox PUBLIC engine-runtime)

set_target_properties(Sandbox PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>")

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
  <cstring>
  <source_location>
  <fmt/core.h>
  <unordered_map>
  <map>
  <functional>
  <string_view>
  <string>
  <type_traits>
  <cstdint>
  <cstddef>
)