cmake_minimum_required(VERSION 3.16)
project(clarplarp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
  gurt.cpp
  Renderer/Renderer.cpp
  Math/Matrix4.cpp
  Core/Timer.cpp
  Core/Camera.cpp
  IO/OBJLoader.cpp
)

add_executable(gurt ${SOURCES}
  Core/Camera.cpp
  Math/Vector4.h
  Graphics/Plane.h
  IO/OBJLoader.cpp
  Graphics/Light.h
  Graphics/Texture.h
  Graphics/Texture.cpp)
target_include_directories(gurt PRIVATE ${CMAKE_SOURCE_DIR})

# Try to find both using CMake Config files first (QUIETLY, so it doesn't crash)
find_package(SDL3 CONFIG QUIET)
find_package(SDL3_ttf CONFIG QUIET)
find_package(SDL3_image CONFIG QUIET)

if (SDL3_FOUND AND SDL3_ttf_FOUND AND SDL3_image_FOUND)
  # Happy path: Both found via CMake Config
  target_link_libraries(gurt PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf SDL3_image::SDL3_image)
else()
  # Fallback path: Use PkgConfig (common on Linux/MSYS2)
  message(STATUS "CMake config not found, falling back to PkgConfig...")
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(SDL3_PKG REQUIRED sdl3)
  pkg_check_modules(SDL3_TTF_PKG REQUIRED sdl3-ttf)
  pkg_check_modules(SDL3_IMAGE_PKG REQUIRED sdl3-image)
  
  target_include_directories(gurt PRIVATE ${SDL3_PKG_INCLUDE_DIRS} ${SDL3_TTF_PKG_INCLUDE_DIRS} ${SDL3_IMAGE_PKG_INCLUDE_DIRS})
  target_link_libraries(gurt PRIVATE ${SDL3_PKG_LIBRARIES} ${SDL3_TTF_PKG_LIBRARIES} ${SDL3_IMAGE_PKG_LIBRARIES})
  target_compile_options(gurt PRIVATE ${SDL3_PKG_CFLAGS_OTHER} ${SDL3_TTF_PKG_CFLAGS_OTHER} ${SDL3_IMAGE_PKG_CFLAGS_OTHER})
endif()

target_compile_options(gurt PRIVATE -Wall -Wextra)

# Copy DLLs for Windows/MinGW environments
if (TARGET SDL3::SDL3-shared)
    add_custom_command(TARGET gurt POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3-shared>
            $<TARGET_FILE_DIR:gurt>
    )
endif()

if(MINGW)
    # Copy all essential SDL3, SDL3_ttf, and font rendering dependency DLLs
    add_custom_command(TARGET gurt POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "C:/msys64/ucrt64/bin/SDL3.dll"
            "C:/msys64/ucrt64/bin/SDL3_ttf.dll"
            "C:/msys64/ucrt64/bin/SDL3_image.dll"
            "C:/msys64/ucrt64/bin/libfreetype-6.dll"
            "C:/msys64/ucrt64/bin/libharfbuzz-0.dll"
            "C:/msys64/ucrt64/bin/libglib-2.0-0.dll"
            "C:/msys64/ucrt64/bin/libintl-8.dll"
            "C:/msys64/ucrt64/bin/libiconv-2.dll"
            "C:/msys64/ucrt64/bin/libpcre2-8-0.dll"
            "C:/msys64/ucrt64/bin/libgraphite2.dll"
            "C:/msys64/ucrt64/bin/libbrotlicommon.dll"
            "C:/msys64/ucrt64/bin/libbrotlidec.dll"
            "C:/msys64/ucrt64/bin/libbz2-1.dll"
            "C:/msys64/ucrt64/bin/libpng16-16.dll"
            "C:/msys64/ucrt64/bin/libzstd.dll"
            "C:/msys64/ucrt64/bin/zlib1.dll"
            "C:/msys64/ucrt64/bin/libstdc++-6.dll"
            "C:/msys64/ucrt64/bin/libgcc_s_seh-1.dll"
            "C:/msys64/ucrt64/bin/libwinpthread-1.dll"
            $<TARGET_FILE_DIR:gurt>
    )
endif()