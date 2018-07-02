# find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

include_directories(${nucommander_SOURCE_DIR}/src
                    ${Boost_INCLUDE_DIRS})
                    
add_definitions(-DBOOST_TEST_DYN_LINK)

## Get source files of "paths" component

get_property(NUC_PATHS GLOBAL PROPERTY PATHS_SRCS)
add_executable(Test path_utils_test.cpp ${NUC_PATHS})

target_link_libraries(Test
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
                      
add_test(NAME NuCommanderTest COMMAND Test)
