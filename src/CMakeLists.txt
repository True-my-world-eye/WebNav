# ============================================================
# src/CMakeLists.txt — 源代码目录入口
# ============================================================

add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(utils)
add_subdirectory(app)

# ── 主程序可执行文件 ──
qt_add_executable(WebNav
    main.cpp
    ${CMAKE_SOURCE_DIR}/resources/resources.qrc
)

target_link_libraries(WebNav PRIVATE
    WebNav_app
    WebNav_ui
    WebNav_core
    WebNav_utils
)

# ── Windows 专属设置 ──
if(WIN32)
    set_target_properties(WebNav PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
