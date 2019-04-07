cmake_minimum_required(VERSION 3.12)
project(libyara-wasm)

set(CMAKE_CXX_STANDARD 14)

include(ExternalProject)

include_directories(
        ${CMAKE_CURRENT_BINARY_DIR}/bzip2-prefix/src/yara/bzip2
        ${CMAKE_CURRENT_SOURCE_DIR}
        $ENV{EMSCRIPTEN}/system/include/
)

ExternalProject_Add(bzip2
        GIT_REPOSITORY https://salsa.debian.org/debian/bzip2.git
        GIT_TAG 0c9335755bcd9519bfc36cf57eb987603b013f46
        CONFIGURE_COMMAND ""
        COMMAND sed -i "s|^CC=gcc$||g" Makefile
        COMMAND sed -i "s|^PREFIX=.*$|PREFIX=$ENV{EMSCRIPTEN}/system|g" Makefile
        BUILD_COMMAND $(MAKE) bzip2
        BUILD_IN_SOURCE 1
        BUILD_BYPRODUCTS
        <SOURCE_DIR>/libbz2.a
        INSTALL_COMMAND $(MAKE) install
        )
add_library(libbz2 STATIC IMPORTED GLOBAL)
set_target_properties(libbz2
        PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/bzip2-prefix/src/bzip2/libbz2.a
        )
add_dependencies(libbz2 bzip2)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../dist)
add_executable(libbzip2-wasm main.cpp)
add_dependencies(libbzip2-wasm libbz2)
target_link_libraries(libbzip2-wasm libbz2)
set_target_properties(libbzip2-wasm
        PROPERTIES SUFFIX ".js"
        LINK_FLAGS "--bind -O3 -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1 -s NO_FILESYSTEM=1 -s SINGLE_FILE=1 -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s ENVIRONMENT='node, worker'"
)
