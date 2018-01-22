cmake_minimum_required(VERSION 2.6)
project(nucommander)

set (CMAKE_CXX_STANDARD 11)

macro (add_sources)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND SRCS "${_relPath}/${_src}")
        else()
            list (APPEND SRCS "${_src}")
        endif()
    endforeach()
    if (_relPath)
        # propagate SRCS to parent directory
        set (SRCS ${SRCS} PARENT_SCOPE)
    endif()
endmacro()


find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-3.0)

# Now the variables GTKMM_INCLUDE_DIRS, GTKMM_LIBRARY_DIRS and GTKMM_LIBRARIES 
# contain what you expect 


link_directories(${GTKMM_LIBRARY_DIRS})
include_directories(${GTKMM_INCLUDE_DIRS})

add_subdirectory(src)

set(RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/resources.c)

add_custom_command(OUTPUT ${RESOURCES}
    COMMAND glib-compile-resources --target=resources.c --generate-source nucommander.gresource.xml
    DEPENDS src/window.glade src/fileview.glade src/nucommander.gresource.xml
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_sources(${RESOURCES})

add_executable(nucommander ${SRCS})
target_link_libraries(nucommander ${GTKMM_LIBRARIES})

install(TARGETS nucommander RUNTIME DESTINATION bin)
