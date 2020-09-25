add_executable(${MAIN_BINARY})

target_include_directories(${MAIN_BINARY} PUBLIC include)
target_sources(${MAIN_BINARY} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/main.cpp)