add_executable(Sandbox)

option(BUILD_SHARED_LIBS OFF)

target_sources(Sandbox PRIVATE src/main.cpp)

target_link_libraries(Sandbox PUBLIC engine-runtime)

set_target_properties(Sandbox PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>")