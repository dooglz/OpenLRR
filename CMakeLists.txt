cmake_minimum_required (VERSION 3.13)

project (OpenLRR VERSION 1.0 LANGUAGES CXX)

file(GLOB_RECURSE sourcefiles src/*.cpp src/*.h)
add_executable(OpenLRR 
${sourcefiles}
)
target_compile_features(OpenLRR PUBLIC cxx_std_17)
set_target_properties(OpenLRR PROPERTIES CXX_EXTENSIONS ON)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "OpenLRR" )
#set(VS_DEBUGGER_WORKING_DIRECTORY  ${CMAKE_BINARY_DIR}/$<CONFIGURATION>)
set_target_properties(OpenLRR PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY $<CONFIG>)

set_target_properties(OpenLRR PROPERTIES 
	VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:OpenLRR>"
	VS_DEBUGGER_COMMAND           "$<TARGET_FILE:OpenLRR>"
	VS_DEBUGGER_ENVIRONMENT       "PATH=%PATH%;${CMAKE_PREFIX_PATH}/bin"
)

#set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(root ${CMAKE_CURRENT_SOURCE_DIR}/src)
source_group(engine REGULAR_EXPRESSION "^.*src[\\|/]engine.*$")
source_group(engine\\graphics REGULAR_EXPRESSION "^.*src[\\|/]engine[\\|/]graphics.*$")
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
add_subdirectory(deps/glm EXCLUDE_FROM_ALL)
add_library(fastNoise deps/fastnoise/fastnoise.cpp deps/fastnoise/fastnoise.h)
target_include_directories(fastNoise SYSTEM PUBLIC deps/fastnoise)
target_compile_features(fastNoise PUBLIC cxx_std_17)

target_link_libraries (OpenLRR Vulkan::Vulkan glfw glm::glm fastNoise)
#target_include_directories(OpenLRR SYSTEM PRIVATE Vulkan::Vulkan)


find_program(GLSLC glslc )
message(${GLSLC})
if(EXISTS "${Vulkan_INCLUDE_DIR}/../Bin32/glslc")
 get_filename_component(GLSLC "${Vulkan_INCLUDE_DIR}/../Bin32/glslc" ABSOLUTE)
else()
 message("Can't find glslc" )
endif()

file(GLOB shaderfiles ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/graphics/shaders/*.glsl.*)
set(ShaderOutputDir ${CMAKE_BINARY_DIR}/$<CONFIGURATION>/res/shaders/)
foreach(loop_var ${shaderfiles})
	  string(REGEX REPLACE "(\.glsl|^.*shaders\/)" "" stripfile ${loop_var})
	  set(spvout ${ShaderOutputDir}${stripfile}.spv)
	  set(shaderCMD ${shaderCMD} ${GLSLC} $<SHELL_PATH:${loop_var}> -o $<SHELL_PATH:${spvout}> \r\n\t)
endforeach(loop_var)
message(${GLSLC})
message(${shaderCMD})
add_custom_target(CompileShaders ALL 
	COMMAND ${CMAKE_COMMAND} -E make_directory ${ShaderOutputDir}
	COMMAND ${shaderCMD}
	COMMAND_EXPAND_LISTS
		SOURCES ${shaderfiles}
)
add_dependencies(OpenLRR CompileShaders)



