cmake_minimum_required(VERSION 3.16)
project(clarplarp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
  gurt.cpp
  Renderer/Renderer.cpp
)

add_executable(gurt ${SOURCES})
target_include_directories(gurt PRIVATE ${CMAKE_SOURCE_DIR})

find_package(SDL3 CONFIG QUIET)
if (SDL3_FOUND)
  target_link_libraries(gurt PRIVATE SDL3::SDL3)
else()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(SDL3_PKG REQUIRED sdl3)
  target_include_directories(gurt PRIVATE ${SDL3_PKG_INCLUDE_DIRS})
  target_link_libraries(gurt PRIVATE ${SDL3_PKG_LIBRARIES})
  target_compile_options(gurt PRIVATE ${SDL3_PKG_CFLAGS_OTHER})
endif()

target_compile_options(gurt PRIVATE -Wall -Wextra)

if (TARGET SDL3::SDL3-shared)
    add_custom_command(TARGET gurt POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3-shared>
            $<TARGET_FILE_DIR:gurt>
    )
endif()

if(MINGW)
    add_custom_command(TARGET gurt POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "C:/msys64/ucrt64/bin/SDL3.dll"
            "C:/msys64/ucrt64/bin/libstdc++-6.dll"
            "C:/msys64/ucrt64/bin/libgcc_s_seh-1.dll"
            "C:/msys64/ucrt64/bin/libwinpthread-1.dll"
            $<TARGET_FILE_DIR:gurt>
    )
endif()