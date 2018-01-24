# find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

include_directories(${nucommander_SOURCE_DIR}/src
                    ${Boost_INCLUDE_DIRS})
                    
add_definitions(-DBOOST_TEST_DYN_LINK)

set(NUC_PATH_UTILS ../src/path_utils.h
                   ../src/path_utils.cpp
                   ../src/path_components.h
                   ../src/path_components.cpp)

add_definitions (-DBOOST_TEST_DYN_LINK)
add_executable(Test path_utils_test.cpp ${NUC_PATH_UTILS})

target_link_libraries(Test
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
                      
add_test(NAME NuCommanderTest COMMAND Test)