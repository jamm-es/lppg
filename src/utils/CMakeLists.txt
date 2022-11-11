add_library(utils "")

target_sources(utils
    PRIVATE
    PoissonDiskSampler.cpp
    Triangulation.cpp
    ChunkCoord.cpp
    PUBLIC
    PoissonDiskSampler.h
    Triangulation.h
    ChunkCoord.h
    )

target_include_directories(utils
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    )

target_link_libraries(utils
    PRIVATE
    raylib
    delaunator
    )