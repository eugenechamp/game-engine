add_library(engine-runtime)

set(HEADERS 
  runtime/include/runtime/app.hpp
  runtime/include/runtime/subsystem.hpp
  runtime/include/runtime/fwd.hpp
  runtime/include/runtime/events.hpp
  runtime/include/runtime/commands.hpp
  runtime/include/runtime/world.hpp
  runtime/include/runtime/def.hpp


  #temps
  runtime/include/runtime/window.hpp
  runtime/include/runtime/meta.hpp
  runtime/include/runtime/batcher.hpp
  runtime/include/runtime/type_traits.hpp

  runtime/include/runtime/log.hpp

  runtime/include/runtime/camera.hpp
  rendering/mesh.hpp
  rendering/model.hpp
  rendering/shader.hpp
  rendering/gfx.hpp
  io.hpp 
)

set(SOURCES
  "runtime/src/app.cpp"
  "runtime/src/subsystem.cpp"
  "runtime/src/commands.cpp"
  "runtime/src/events.cpp"

  rendering/mesh.cpp
  rendering/model.cpp
  rendering/shader.cpp
  rendering/gfx.cpp
  io.cpp 
)

target_sources(engine-runtime PRIVATE ${SOURCES} ${HEADERS})

target_include_directories(engine-runtime PUBLIC runtime/include PUBLIC .)

target_link_libraries(
  engine-runtime 
  PUBLIC flecs 
  PUBLIC SDL3::SDL3 
  PUBLIC fmt::fmt
  PUBLIC libimgui
  PUBLIC libglad
  PUBLIC glm::glm
  PUBLIC assimp
  PUBLIC stblib
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

target_precompile_headers(engine-runtime
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
