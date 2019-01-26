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
    
    cleanup_n.h
)

add_subdirectory(paths/)
add_subdirectory(plugins/)
add_subdirectory(errors/)
add_subdirectory(tasks/)
add_subdirectory(lister/)
add_subdirectory(directory/)
add_subdirectory(stream/)
add_subdirectory(operations/)
add_subdirectory(commands/)
add_subdirectory(settings/)
add_subdirectory(interface/)
add_subdirectory(util/)
add_subdirectory(file_list/)
