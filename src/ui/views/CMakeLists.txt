# ui/views/CMakeLists.txt
# 数据展示视图
qt_add_library(WebNav_ui_views STATIC
    LinkListView.h    LinkListView.cpp
    LinkCardView.h    LinkCardView.cpp
)
target_link_libraries(WebNav_ui_views PUBLIC
    Qt6::Core
    Qt6::Widgets
    WebNav_core
    WebNav_utils
)
target_include_directories(WebNav_ui_views PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
