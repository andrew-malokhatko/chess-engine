cmake_minimum_required (VERSION 3.24)

project (chess-engine LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 23)

# Adding Raylib
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raylib)

find_package(raylib REQUIRED)

add_subdirectory(Chess/Core)
add_subdirectory(Chess/AI)
add_subdirectory(Tests)
add_subdirectory(Chess)

add_subdirectory(UI/Clipboard)
add_subdirectory(UI/Components)
add_subdirectory(UI)


add_executable(engine chess-engine.cpp)

target_link_libraries(engine
    Chess
	Tests
	UI
    raylib
)

target_compile_definitions(engine PUBLIC IMAGES_PATH=./Resources/Images/)
target_compile_definitions(engine PUBLIC GAMES_PATH=./Resources/Games/)
target_compile_definitions(engine PUBLIC SOUNDS_PATH=./Resources/Sounds/)

# Copy resources folder to build directory
add_custom_target(CopyResources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Resources  # Source directory
    ${CMAKE_BINARY_DIR}/Resources  # Destination directory
)

add_dependencies(engine CopyResources)