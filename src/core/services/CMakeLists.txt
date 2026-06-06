# core/services/CMakeLists.txt
# 业务服务
qt_add_library(WebNav_core_services STATIC
    CryptoService.h      CryptoService.cpp
    FaviconService.h     FaviconService.cpp
    LinkCheckService.h   LinkCheckService.cpp
    BookmarkImporter.h   BookmarkImporter.cpp
    BookmarkExporter.h   BookmarkExporter.cpp
)
target_link_libraries(WebNav_core_services PUBLIC
    Qt6::Core
    Qt6::Network
    WebNav_core_models
    WebNav_core_database
)
target_include_directories(WebNav_core_services PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
