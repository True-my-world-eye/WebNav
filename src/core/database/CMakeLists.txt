# core/database/CMakeLists.txt
# 数据持久化 — 接口 + SQLite 实现
qt_add_library(WebNav_core_database STATIC
    DatabaseManager.h        DatabaseManager.cpp
    interfaces/ILinkRepository.h
    interfaces/IFolderRepository.h
    interfaces/ITagRepository.h
    impl/SqliteLinkRepository.h    impl/SqliteLinkRepository.cpp
    impl/SqliteFolderRepository.h  impl/SqliteFolderRepository.cpp
    impl/SqliteTagRepository.h     impl/SqliteTagRepository.cpp
)
target_link_libraries(WebNav_core_database PUBLIC
    Qt6::Core
    Qt6::Sql
    WebNav_core_models
)
target_include_directories(WebNav_core_database PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
