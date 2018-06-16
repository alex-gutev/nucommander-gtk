pkg_check_modules(LIBARCHIVE libarchive)

include_directories(${LIBARCHIVE_INCLUDE_DIRS})
link_directories(${LIBARCHIVE_LIBRARY_DIRS})

set(ARCH_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/plugin.c
                 ${CMAKE_CURRENT_SOURCE_DIR}/../../archive_plugin_types.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/../../archive_plugin_api.h)
                 
add_library(archgeneric SHARED ${ARCH_SOURCES})

target_link_libraries(archgeneric ${LIBARCHIVE_LIBRARIES})
