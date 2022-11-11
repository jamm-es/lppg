add_executable(${PROJECT_NAME} main.cpp)

add_subdirectory(utils)
add_subdirectory(terrain)
add_subdirectory(chunk_manager)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        raylib
        fastnoiselite
        delaunator
        utils
        terrain
        chunk_manager
    )