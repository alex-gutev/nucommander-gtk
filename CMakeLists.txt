cmake_minimum_required(VERSION 2.6)
project(nucommander)

set (CMAKE_CXX_STANDARD 11)

## Macros

# Adds the full file path of files relative to
# CMAKE_CURRENT_SOURCE_DIR, passed after the last required argument,
# to the global property SRC_VAR. A copy of the list stored in the
# property SRC_VAR is also stored in the local variable SRC_VAR.

macro (add_sources SRC_VAR)
  # Get global value of SRC_VAR
  get_property(${SRC_VAR} GLOBAL PROPERTY ${SRC_VAR})

  foreach (_src ${ARGN})
    list (APPEND ${SRC_VAR} "${CMAKE_CURRENT_SOURCE_DIR}/${_src}")
  endforeach()

  # Set value of global property SRC_VAR
  set_property(GLOBAL PROPERTY ${SRC_VAR} ${${SRC_VAR}})
endmacro()

# Appends the file list stored in SRC_VAR to the list stored in the
# global property DEST_VAR.

macro (append_sources DEST_VAR SRC_VAR)
  get_property(${DEST_VAR} GLOBAL PROPERTY ${DEST_VAR})
  set(${DEST_VAR} ${${DEST_VAR}} ${${SRC_VAR}})
  set_property(GLOBAL PROPERTY ${DEST_VAR} ${${DEST_VAR}})
endmacro()


## Dependencies

find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-3.0)

link_directories(${GTKMM_LIBRARY_DIRS})
include_directories(${GTKMM_INCLUDE_DIRS})


## Add source directories

add_subdirectory(src)

# Convert gresource.xml files to C, output resources.c

add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/resources.c
    COMMAND glib-compile-resources --target=resources.c --generate-source nucommander.gresource.xml
    DEPENDS src/window.glade src/fileview.glade src/nucommander.gresource.xml
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_sources(NUC_SRCS src/resources.c)


# Compile schemas and move to target directory
  
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gschemas.compiled
    COMMAND glib-compile-schemas --strict --targetdir=${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src
    DEPENDS src/nucommander.gschema.xml)

add_custom_target(gsettings_schema ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/gschemas.compiled)



# NuCommander executable

get_property(SRCS GLOBAL PROPERTY NUC_SRCS)

add_executable(nucommander ${SRCS})
add_dependencies(nucommander gsettings_schema archgeneric)

target_link_libraries(nucommander ${GTKMM_LIBRARIES} dl)

install(TARGETS nucommander RUNTIME DESTINATION bin)


## Tests

enable_testing()
add_subdirectory(test)
