file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    *.cpp
    *.h
)

add_executable(TimeThiefServer ${APP_SOURCES})

target_link_libraries(TimeThiefServer PRIVATE ServerEngineLib)

target_include_directories(TimeThiefServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(TimeThiefServer PRIVATE
    Utils/pch.h
)
