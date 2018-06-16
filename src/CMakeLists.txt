add_sources(
    main.cpp 
    
    nucommander.cpp 
    nucommander.h
    
    app_window.cpp
    app_window.h
    
    file_view.h
    file_view.cpp
    
    file_list_controller.h
    file_list_controller.cpp
    
    file_model_columns.h
    file_model_columns.cpp
    
    types.h
    
    async_queue.h
    async_task.h
    async_task.cpp
    
    lister.h
    dir_lister.h
    dir_lister.cpp
    
    path_components.h
    path_components.cpp
    
    path_utils.h
    path_utils.cpp
    
    dir_tree.h
    dir_tree.cpp
    
    dir_entry.h
    dir_entry.cpp
    
    cancel_state.h
    cancel_state.cpp
    cleanup_n.h
    
    operation.h
    operation.cpp
    fn_operation.h
    
    task_queue.h
    task_queue.cpp
    
    vfs.h
    vfs.cpp
    
    dir_monitor.h
    dir_monitor.cpp
    
    archive_plugin_types.h
    archive_plugin.h
    archive_plugin.cpp
    
    archive_plugin_loader.h
    archive_plugin_loader.cpp
    
    dir_type.h
    dir_type.cpp
    
    archive_lister.h
    archive_lister.cpp
    
    archive_tree.h
    archive_tree.cpp
)

# Add current directory to include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(plugins/)
