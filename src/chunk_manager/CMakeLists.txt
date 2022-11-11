add_library(chunk_manager "")

target_sources(chunk_manager
    PRIVATE
    ChunkManager.cpp
    PUBLIC
    ChunkManager.h
    )

target_include_directories(chunk_manager
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    )

target_link_libraries(chunk_manager
    PRIVATE
    raylib
    utils
    terrain
    )