add_library(libglad STATIC)

set(SOURCES src/glad.c include/glad/glad.h include/KHR/khrplatform.h)
target_sources(libglad PRIVATE ${SOURCES})

target_include_directories(libglad PUBLIC include)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
