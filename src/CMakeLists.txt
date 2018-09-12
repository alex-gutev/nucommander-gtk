add_sources(NUC_SRCS
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
    
    error.h
    error.cpp

    async_queue.h
    async_task.h
    async_task.cpp
    
    dir_tree.h
    dir_tree.cpp
    
    dir_entry.h
    dir_entry.cpp
    
    cancel_state.h
    cancel_state.cpp
    cleanup_n.h
    
    task_queue.h
    task_queue.cpp
    
    vfs.h
    vfs.cpp
    
    dir_monitor.h
    dir_monitor.cpp
    
    dir_type.h
    dir_type.cpp
    
    archive_tree.h
    archive_tree.cpp

    icon_loader.h
    icon_loader.cpp
)

add_subdirectory(paths/)
add_subdirectory(plugins/)
add_subdirectory(lister/)
add_subdirectory(stream/)
