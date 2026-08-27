cmake_minimum_required(VERSION 3.20)

project(Renderer2D)

# C++ config
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(SRC_DIR "project/src")

add_library(${PROJECT_NAME} STATIC
	"${SRC_DIR}/Renderer2D.cpp"
	"${SRC_DIR}/GraphicsContext.cpp"
	"${SRC_DIR}/Shader.cpp"
	"${SRC_DIR}/Pipeline.cpp"
	"${SRC_DIR}/DataTypes.cpp"
	"${SRC_DIR}/BindGroupLayout.cpp"
	"${SRC_DIR}/RenderQueue.cpp"
	"${SRC_DIR}/Material.cpp"
	"${SRC_DIR}/Texture2D.cpp"
	"${SRC_DIR}/Sampler.cpp"
	"${SRC_DIR}/Camera2D.cpp"
	"${SRC_DIR}/BuiltinResources.cpp"
)

# need this for fetching
add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS ${PROJECT_NAME})

# include directories
set(LIB_DIR "project/lib")
set(INC_DIR "project/inc")

target_include_directories(${PROJECT_NAME} PUBLIC
	"${LIB_DIR}/glm"
	"${LIB_DIR}/webgpu/include"
	"${INC_DIR}"
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# link libraries
set(SDL3_DIR "${LIB_DIR}/SDL3/cmake")
set(SDL3_image_DIR "${LIB_DIR}/SDL3_image/cmake")

find_package(SDL3 REQUIRED GLOBAL)
find_package(SDL3_image REQUIRED GLOBAL)

# subdirectories
add_subdirectory("${LIB_DIR}/webgpu")
add_subdirectory("${LIB_DIR}/sdl3webgpu")

add_compile_definitions(IMGUI_IMPL_WEBGPU_BACKEND_WGPU)

# ImGui
set(IMGUI_DIR "${LIB_DIR}/imgui")
add_library(imgui STATIC
    "${IMGUI_DIR}/backends/imgui_impl_wgpu.h"
    "${IMGUI_DIR}/backends/imgui_impl_wgpu.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdl3.h"
    "${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp"

    "${IMGUI_DIR}/misc/cpp/imgui_stdlib.h"
    "${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp"

    "${IMGUI_DIR}/imconfig.h"
    "${IMGUI_DIR}/imgui.h"
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_internal.h"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/imstb_rectpack.h"
    "${IMGUI_DIR}/imstb_textedit.h"
    "${IMGUI_DIR}/imstb_truetype.h"
)

target_include_directories(imgui PUBLIC ${IMGUI_DIR})
target_link_libraries(imgui PUBLIC webgpu SDL3::SDL3)

target_link_libraries(${PROJECT_NAME} PUBLIC
	webgpu
	SDL3::SDL3
	SDL3_image::SDL3_image
	sdl3webgpu
	imgui
)

# set warning level to 4 and make warnings cause errors
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

function(${PROJECT_NAME}_Copy_Dependencies TARGET)

	# copy dll's next to library
	target_copy_webgpu_binaries(${TARGET})

	add_custom_command(TARGET ${TARGET} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"$<TARGET_FILE:SDL3::SDL3>"
		"$<TARGET_FILE_DIR:${TARGET}>"
	)
	add_custom_command(TARGET ${TARGET} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"$<TARGET_FILE:SDL3_image::SDL3_image>"
		"$<TARGET_FILE_DIR:${TARGET}>"
	)

	message("Copied ${PROJECT_NAME}'s dependencies next to ${TARGET}")

endfunction()

add_executable(example "examples/main.cpp")

target_link_libraries(example PRIVATE 
	Renderer2D::Renderer2D
	SDL3::SDL3
)

Renderer2D_Copy_Dependencies(example)

# copy resources
add_custom_command(
	TARGET example POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
	"${CMAKE_SOURCE_DIR}/resources"
	"$<TARGET_FILE_DIR:example>/resources"
)