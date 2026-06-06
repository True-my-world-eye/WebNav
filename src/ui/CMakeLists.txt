# ui/CMakeLists.txt
# UI 层入口
add_subdirectory(widgets)
add_subdirectory(views)
add_subdirectory(dialogs)

qt_add_library(WebNav_ui STATIC
    MainWindow.h   MainWindow.cpp
)
target_link_libraries(WebNav_ui PUBLIC
    Qt6::Core
    Qt6::Widgets
    WebNav_core
    WebNav_ui_widgets
    WebNav_ui_views
    WebNav_ui_dialogs
)
target_include_directories(WebNav_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
