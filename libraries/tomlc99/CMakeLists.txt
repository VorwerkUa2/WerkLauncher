project(tomlc99)

set(tomlc99_SOURCES
include/toml.h
src/toml.c
)

add_library(tomlc99 STATIC ${tomlc99_SOURCES})

target_include_directories(tomlc99 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(MSVC)
    target_compile_options(tomlc99 PRIVATE /wd4267 /wd4244 /wd4996)
endif()
