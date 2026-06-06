# core/sync/CMakeLists.txt
# 同步模块（Phase 3 实现，Phase 1 仅占位）
qt_add_library(WebNav_core_sync STATIC
    ISyncProvider.h
    SyncManager.h        SyncManager.cpp
)
target_link_libraries(WebNav_core_sync PUBLIC
    Qt6::Core
    Qt6::Network
    WebNav_core_models
    WebNav_core_database
)
target_include_directories(WebNav_core_sync PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
