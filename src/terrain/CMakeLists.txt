add_library(terrain "")

target_sources(terrain
    PRIVATE
        Terrain.cpp
    PUBLIC
        Terrain.h
    )

target_include_directories(terrain
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    )

target_link_libraries(terrain
    PRIVATE
        raylib
        fastnoiselite
        utils
        chunk_manager
    )