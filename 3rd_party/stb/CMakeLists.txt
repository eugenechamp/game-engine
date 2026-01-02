add_library(stblib STATIC)

set(STB_SRC 
  include/stb_image.h 
  src/stb_image.cpp
)

target_sources(stblib PRIVATE ${STB_SRC})

target_include_directories(stblib PUBLIC include)

if(MSVC)
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${stb_sources})
endif(MSVC)