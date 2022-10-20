add_executable(${PROJECT_NAME} main.cpp)

add_subdirectory(utils)

add_subdirectory(terrain)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        raylib
        fastnoiselite
        terrain
        utils
    )