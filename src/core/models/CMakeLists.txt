# core/models/CMakeLists.txt
# 数据模型（纯 struct）
qt_add_library(WebNav_core_models STATIC
    Link.h
    Folder.h
    Tag.h
    LinkField.h
)
target_include_directories(WebNav_core_models PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
