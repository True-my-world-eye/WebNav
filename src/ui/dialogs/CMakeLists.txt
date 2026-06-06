# ui/dialogs/CMakeLists.txt
# 弹窗对话框
qt_add_library(WebNav_ui_dialogs STATIC
    LinkEditDialog.h    LinkEditDialog.cpp
    SettingsDialog.h    SettingsDialog.cpp
    SyncConfigDialog.h  SyncConfigDialog.cpp
    AboutDialog.h       AboutDialog.cpp
    QRCodeDialog.h      QRCodeDialog.cpp
)
target_link_libraries(WebNav_ui_dialogs PUBLIC
    Qt6::Core
    Qt6::Widgets
    WebNav_core
    WebNav_ui_widgets
)
target_include_directories(WebNav_ui_dialogs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
