include(FetchContent)

FetchContent_Declare(
  SDL
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG        release-3.2.2 # release-1.10.0
)
FetchContent_MakeAvailable(SDL)

FetchContent_Declare(
  flecs
  GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
  GIT_TAG v4.0.5
)
FetchContent_MakeAvailable(flecs)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 12.0.0
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.92.5-docking
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.3
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
  libassimp
  GIT_REPOSITORY https://github.com/assimp/assimp.git
  GIT_TAG v6.0.2
)
FetchContent_MakeAvailable(libassimp)

add_library(libimgui STATIC)

target_include_directories(libimgui PUBLIC "../build/_deps/imgui-src")

set(IMGUI_SOURCES 
  "../build/_deps/imgui-src/imconfig.h"
  "../build/_deps/imgui-src/imgui.h"
  "../build/_deps/imgui-src/imgui.cpp"
  "../build/_deps/imgui-src/imgui_draw.cpp"
  "../build/_deps/imgui-src/imgui_tables.cpp"
  "../build/_deps/imgui-src/imgui_internal.h"
  "../build/_deps/imgui-src/imgui_widgets.cpp"
  "../build/_deps/imgui-src/imstb_rectpack.h"
  "../build/_deps/imgui-src/imstb_textedit.h"
  "../build/_deps/imgui-src/imstb_truetype.h"
  "../build/_deps/imgui-src/imgui_demo.cpp"

  "../build/_deps/imgui-src/backends/imgui_impl_sdl3.h"
  "../build/_deps/imgui-src/backends/imgui_impl_sdl3.cpp"
  
  "../build/_deps/imgui-src/backends/imgui_impl_opengl3.h"
  "../build/_deps/imgui-src/backends/imgui_impl_opengl3_loader.h"
  "../build/_deps/imgui-src/backends/imgui_impl_opengl3.cpp"
  
  
  # #if I decide to move on to glfw for window creation I will need these 
  # "../build/_deps/imgui-src/backends/imgui_impl_glfw.h"
  # "../build/_deps/imgui-src/backends/imgui_impl_glfw.cpp"

  "../build/_deps/imgui-src/imstb_rectpack.h"
  "../build/_deps/imgui-src/imstb_textedit"
  "../build/_deps/imgui-src/imstb_truetype"
)

target_sources(libimgui PRIVATE ${IMGUI_SOURCES})
target_link_libraries(libimgui PUBLIC SDL3::SDL3 PUBLIC opengl32)

add_subdirectory(glad)
add_subdirectory(stb)

FetchContent_Declare(
  Jolt
  GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics.git
  GIT_TAG v5.5.0
)
FetchContent_MakeAvailable(Jolt)

if(SDL_SHARED)
  set_target_properties(SDL3-shared PROPERTIES 
    FOLDER "Deps"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>")
  
  target_compile_options(SDL3-shared  PRIVATE /MP /W4 /EHsc- /GR-)
else()
  set_target_properties(SDL3-static PROPERTIES 
    FOLDER "Deps"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>")
endif()

set_target_properties(flecs PROPERTIES 
  FOLDER "Deps"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>"
)

set_target_properties(fmt PROPERTIES 
  FOLDER "Deps"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>"
)

set_target_properties(assimp PROPERTIES 
  FOLDER "Deps"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>"
)

if(MSVC)
  target_compile_options(flecs 
    PUBLIC /MP 
    /GR-
  )
  target_compile_options(fmt PUBLIC
    /MP 
    /W4 
    /GR-
  )
  
  if(SDL_SHARED)
    target_compile_options(SDL3-shared  PUBLIC /MP /EHsc- /GR-)
  else()
    target_compile_options(SDL3-static  PUBLIC /MP /EHsc- /GR-)
  endif()
  
  target_compile_options(libimgui 
    PUBLIC /MP
    /W4
    /GR-
  )
  target_compile_options(libglad 
    PUBLIC /MP
    /W4
    /GR-
  )
  target_compile_options(stblib 
    PUBLIC /MP
    /W4
    /GR-
  )
  target_compile_options(glm 
    PUBLIC /MP
    /W4
    /GR-
  )
else() # additional: -ftime-trace
  target_compile_options(flecs -Wall -fno-rtti -fno-exceptions -Wextra -Wpedantic)
  target_compile_options(fmt -Wall -fno-rtti -fno-exceptions -Wextra -Wpedantic)
  
  if(SDL_SHARED)
    target_compile_options(SDL3-shared 
      PRIVATE -Wall 
      PRIVATE -fno-rtti 
      PRIVATE -fno-exceptions 
      PRIVATE -Wextra 
      PRIVATE -Wpedantic
    )
  else()
    target_compile_options(SDL3-static PRIVATE -Wall -fno-rtti -fno-exceptions -Wextra -Wpedantic)
  endif()

  target_compile_options(libimgui PRIVATE -Wall -fno-rtti -fno-exceptions -Wextra -Wpedantic)
  target_compile_options(libglad PRIVATE -Wall -fno-rtti -fno-exceptions -Wextra -Wpedantic)
  target_compile_options(stblib PRIVATE -Wall -fno-rtti -fno-exceptions -Wextra -Wpedantic)
  target_compile_options(glm PRIVATE -Wall -fno-rtti -fno-exceptions -Wextra -Wpedantic)
endif()



