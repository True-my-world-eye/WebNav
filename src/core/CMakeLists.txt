# core/CMakeLists.txt
# 核心业务层入口
add_subdirectory(models)
add_subdirectory(database)
add_subdirectory(services)
add_subdirectory(sync)

qt_add_library(WebNav_core STATIC "")
target_link_libraries(WebNav_core PUBLIC
    Qt6::Core
    Qt6::Sql
    Qt6::Network
    WebNav_core_models
    WebNav_core_database
    WebNav_core_services
    WebNav_core_sync
)
target_include_directories(WebNav_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
