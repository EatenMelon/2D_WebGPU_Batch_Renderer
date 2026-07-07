cmake_minimum_required(VERSION 3.20)

project(Renderer2D)

# C++ config
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(SRC_DIR "project/src")
add_executable(${PROJECT_NAME}
	"${SRC_DIR}/main.cpp"
	"${SRC_DIR}/Renderer2D.cpp"
	"${SRC_DIR}/GraphicsContext.cpp"
)

# include directories
set(LIB_DIR "project/lib")

target_include_directories(${PROJECT_NAME} PRIVATE
	"${LIB_DIR}/glm"
	"${LIB_DIR}/webgpu/include"
)

# link libraries
set(SDL3_DIR "${LIB_DIR}/SDL3/cmake")
find_package(SDL3 REQUIRED)

if(NOT TARGET SDL3::SDL3-shared)
    message(FATAL_ERROR "SDL3 shared target not found — DLL build not available")
endif()

# subdirectories
add_subdirectory("${LIB_DIR}/webgpu")
add_subdirectory("${LIB_DIR}/sdl3webgpu")

target_link_libraries(${PROJECT_NAME} PRIVATE
	webgpu
	SDL3::SDL3-shared
	sdl3webgpu
)

# set warning level to 4 and make warnings cause errors
target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)

# copy dll's next to executable
target_copy_webgpu_binaries(${PROJECT_NAME})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE:SDL3::SDL3-shared>"
	"$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)

# copy resources
add_custom_command(
	TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
	"${CMAKE_SOURCE_DIR}/resources"
	"$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
)