target_precompile_headers(ServerEngineLib PRIVATE
    Core/PCH/pch.h
)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    *.cpp
    *.h
)

add_library(ServerEngineLib STATIC ${ENGINE_SOURCES})

target_include_directories(ServerEngineLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
