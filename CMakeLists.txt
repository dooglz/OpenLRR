cmake_minimum_required (VERSION 3.13)

project (OpenLRR VERSION 1.0 LANGUAGES CXX)

add_executable(OpenLRR src/main.cpp src/engine/Engine.cpp src/engine/Engine.h src/engine/platform/platform_glfw.cpp src/engine/platform/platform_glfw.h src/engine/graphics/vulkan.cpp src/engine/graphics/vulkan.h src/engine/graphics/graphics_backend.h)


target_compile_features(OpenLRR PUBLIC cxx_std_17)
set_target_properties(OpenLRR PROPERTIES CXX_EXTENSIONS OFF)

find_package(Vulkan)
IF (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
ELSE()
	message(STATUS ${Vulkan_LIBRARY} ${Vulkan_INCLUDE_DIRS})
ENDIF()

add_subdirectory(deps/glfw EXCLUDE_FROM_ALL)


target_link_libraries (OpenLRR ${Vulkan_LIBRARY} glfw)
target_include_directories(OpenLRR SYSTEM PRIVATE ${Vulkan_INCLUDE_DIRS} )