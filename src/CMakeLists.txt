# src/CMakeLists.txt — 源代码目录入口

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

    # 嵌入 .ico 到 exe（设置应用图标和任务栏图标）
    set(APP_ICO "${CMAKE_SOURCE_DIR}/resources/icons/app.ico")
    if(EXISTS "${APP_ICO}")
        # 生成 .rc 文件并嵌入
        set(RC_FILE "${CMAKE_CURRENT_BINARY_DIR}/app_icon.rc")
        file(WRITE "${RC_FILE}" "IDI_ICON1 ICON \"${APP_ICO}\"\n")
        target_sources(WebNav PRIVATE "${RC_FILE}")
    endif()
endif()
