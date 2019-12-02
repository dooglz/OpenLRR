cmake_minimum_required (VERSION 3.13)

project (OpenLRR VERSION 1.0 LANGUAGES CXX)


add_executable(OpenLRR src/main.cpp src/engine/Engine.cpp src/engine/Engine.h src/engine/platform/platform_glfw.cpp src/engine/platform/platform_glfw.h src/engine/graphics/vulkan.cpp src/engine/graphics/vulkan.h src/engine/graphics/graphics_backend.h src/game/game.cpp)


target_compile_features(OpenLRR PUBLIC cxx_std_17)
set_target_properties(OpenLRR PROPERTIES CXX_EXTENSIONS OFF)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "OpenLRR" )
#set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(root ${CMAKE_CURRENT_SOURCE_DIR}/src)
source_group(engine REGULAR_EXPRESSION "^.*src[\\|/]engine.*$")
source_group(game REGULAR_EXPRESSION "^.*src[\\|/]game.*$")

find_package(Vulkan REQUIRED)
message(STATUS ${Vulkan_LIBRARY} ${Vulkan_INCLUDE_DIRS})

if(CMAKE_GENERATOR MATCHES "Visual Studio")
  #windows vulkan platform layer definition
  target_compile_definitions(OpenLRR PRIVATE VK_USE_PLATFORM_WIN32_KHR)
  #Force multiproc build on windows
  if(CMAKE_C_COMPILER_ID MATCHES "MSVC|Intel" OR CMAKE_CXX_COMPILER_ID MATCHES "MSVC|Intel")
    string(APPEND CMAKE_C_FLAGS " /MP")
    string(APPEND CMAKE_CXX_FLAGS " /MP")
  endif()
endif()

add_subdirectory(deps/glfw EXCLUDE_FROM_ALL)

target_link_libraries (OpenLRR Vulkan::Vulkan glfw)
#target_include_directories(OpenLRR SYSTEM PRIVATE Vulkan::Vulkan)