cmake_minimum_required(VERSION 3.10)
project(rainbow)

find_package(Qt6 REQUIRED COMPONENTS Core Gui QUIET)

add_library(Launcher_rainbow STATIC
    src/rainbow.cpp
)
target_include_directories(Launcher_rainbow PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_compile_definitions(Launcher_rainbow PUBLIC RAINBOW_STATIC)
target_link_libraries(Launcher_rainbow Qt6::Core Qt6::Gui)

# Install it
install(
    TARGETS Launcher_rainbow
    RUNTIME DESTINATION ${LIBRARY_DEST_DIR}
    LIBRARY DESTINATION ${LIBRARY_DEST_DIR}
)
