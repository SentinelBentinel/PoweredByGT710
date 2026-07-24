cmake_minimum_required(VERSION 3.16)

project(gurt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "*.cpp"
)

add_executable(gurt ${SOURCES})

target_include_directories(gurt PRIVATE
    ${CMAKE_SOURCE_DIR}
)

find_package(SDL3 CONFIG QUIET)
find_package(SDL3_image CONFIG QUIET)
find_package(SDL3_ttf CONFIG QUIET)

if(SDL3_FOUND AND SDL3_image_FOUND AND SDL3_ttf_FOUND)
    target_link_libraries(gurt PRIVATE
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
    )
else()
    message(STATUS "Falling back to pkg-config")

    find_package(PkgConfig REQUIRED)

    pkg_check_modules(SDL3 REQUIRED sdl3)
    pkg_check_modules(SDL3_IMAGE REQUIRED sdl3-image)
    pkg_check_modules(SDL3_TTF REQUIRED sdl3-ttf)

    target_include_directories(gurt PRIVATE
        ${SDL3_INCLUDE_DIRS}
        ${SDL3_IMAGE_INCLUDE_DIRS}
        ${SDL3_TTF_INCLUDE_DIRS}
    )

    target_link_libraries(gurt PRIVATE
        ${SDL3_LIBRARIES}
        ${SDL3_IMAGE_LIBRARIES}
        ${SDL3_TTF_LIBRARIES}
    )

    target_compile_options(gurt PRIVATE
        ${SDL3_CFLAGS_OTHER}
        ${SDL3_IMAGE_CFLAGS_OTHER}
        ${SDL3_TTF_CFLAGS_OTHER}
    )
endif()

target_compile_options(gurt PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
