# app/CMakeLists.txt
# 应用层 — 初始化、主题、托盘、热键
qt_add_library(WebNav_app STATIC
    Application.h    Application.cpp
    GlobalHotkey.h   GlobalHotkey.cpp
)
target_link_libraries(WebNav_app PUBLIC
    Qt6::Core
    Qt6::Widgets
    WebNav_core
    WebNav_ui
)
target_include_directories(WebNav_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
