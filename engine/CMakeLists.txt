add_library(engine-runtime)

set(HEADERS 
  "runtime/include/runtime/app.hpp"
  "runtime/include/runtime/game_system.hpp"
  "runtime/include/runtime/fwd.hpp"
  "runtime/include/runtime/events.hpp"
  "runtime/include/runtime/commands.hpp"
  "runtime/include/runtime/world.hpp"
  "runtime/include/runtime/def.hpp"
)

set(SOURCES
  "runtime/src/app.cpp"
  "runtime/src/game_system.cpp"
)

target_sources(engine-runtime PRIVATE ${SOURCES} ${HEADERS})

target_include_directories(engine-runtime PUBLIC runtime/include)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})
