cmake_minimum_required (VERSION 3.12)

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
  target_compile_definitions(OpenLRR PRIVATE VK_USE_PLATFORM_WIN32_KHR NOMINMAX)
  #Force multiproc build on windows
  if(CMAKE_C_COMPILER_ID MATCHES "MSVC|Intel" OR CMAKE_CXX_COMPILER_ID MATCHES "MSVC|Intel")
    string(APPEND CMAKE_C_FLAGS " /MP")
    string(APPEND CMAKE_CXX_FLAGS " /MP")
  endif()
endif()



add_subdirectory(deps/glfw EXCLUDE_FROM_ALL)
add_subdirectory(deps/glm EXCLUDE_FROM_ALL)
add_library(fastNoise ${CMAKE_CURRENT_SOURCE_DIR}/deps/fastnoise/FastNoise.cpp ${CMAKE_CURRENT_SOURCE_DIR}/deps/fastnoise/FastNoise.h)
target_include_directories(fastNoise SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/fastnoise)
target_compile_features(fastNoise PUBLIC cxx_std_17)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE  ${CMAKE_CURRENT_SOURCE_DIR}/deps/stb)

add_library(imgui ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/imgui.cpp ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/imgui.h)
target_include_directories(imgui SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui)
target_compile_features(imgui PUBLIC cxx_std_17)


#target_include_directorie, but marks libs as SYSTEM, so no warnings are shown for them
function(target_link_libraries_system target)
  set(libs ${ARGN})
  foreach(lib ${libs})
    get_target_property(lib_include_dirs ${lib} INTERFACE_INCLUDE_DIRECTORIES)
    target_include_directories(${target} SYSTEM PRIVATE ${lib_include_dirs})
    target_link_libraries(${target} ${lib})
  endforeach(lib)
  #this doesn't work for VS, so we gotta do it their way:
  if(CMAKE_GENERATOR MATCHES "Visual Studio")
    target_compile_options(${target} PRIVATE /experimental:external /external:anglebrackets /external:W0)
  endif()
endfunction(target_link_libraries_system)

target_link_libraries_system (OpenLRR Vulkan::Vulkan glfw glm::glm fastNoise stb)
#target_include_directories(OpenLRR SYSTEM PRIVATE Vulkan::Vulkan)


find_program(GLSLC glslc HINTS "$ENV{VULKAN_SDK}/bin" "${Vulkan_INCLUDE_DIR}/../Bin32/glslc")
if(NOT GLSLC)
	 message("Can't find glslc" )
else()
	message(STATUS "GLSLC: ${GLSLC}") 
endif()

file(GLOB shaderfiles ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/graphics/shaders/*.glsl.*)
set(ShaderOutputDir ${CMAKE_BINARY_DIR}/$<CONFIGURATION>/res/shaders/)
foreach(loop_var ${shaderfiles})
	  string(REGEX REPLACE "(\.glsl|^.*shaders\/)" "" stripfile ${loop_var})
	  set(spvout ${ShaderOutputDir}${stripfile}.spv)
	  set(shaderCMD ${shaderCMD} echo Compiling shader: ${loop_var} to: ${spvout} \r\n\t ${GLSLC} $<SHELL_PATH:${loop_var}> -o $<SHELL_PATH:${spvout}> \r\n\t)
endforeach(loop_var)

add_custom_target(CompileShaders ALL 
	COMMAND ${CMAKE_COMMAND} -E make_directory ${ShaderOutputDir}
	COMMAND ${shaderCMD}
	COMMAND_EXPAND_LISTS
		SOURCES ${shaderfiles}
)

if(APPLE)
  add_custom_target(copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/res" ${CMAKE_BINARY_DIR}/$<CONFIGURATION>/res
  )
else()
  add_custom_target(copy_resources ALL 
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/res" ${CMAKE_BINARY_DIR}/$<CONFIGURATION>/res
  )
endif()

add_dependencies(OpenLRR CompileShaders copy_resources)


