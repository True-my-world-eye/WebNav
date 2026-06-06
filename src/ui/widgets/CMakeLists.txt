# ui/widgets/CMakeLists.txt
# 可复用 UI 组件
qt_add_library(WebNav_ui_widgets STATIC
    Sidebar.h           Sidebar.cpp
    SearchBar.h         SearchBar.cpp
    TagSelector.h       TagSelector.cpp
    TagLabel.h          TagLabel.cpp
    FieldEditor.h       FieldEditor.cpp
    LinkPreviewPanel.h  LinkPreviewPanel.cpp
)
target_link_libraries(WebNav_ui_widgets PUBLIC
    Qt6::Core
    Qt6::Widgets
    WebNav_core
)
target_include_directories(WebNav_ui_widgets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
