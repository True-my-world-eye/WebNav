cmake_minimum_required(VERSION 3.20)
project(WebNav VERSION 1.0.0 LANGUAGES CXX)

# ============================================================
# WebNav - 桌面网页链接管理器
# 基于 Qt 6 的桌面书签管理应用
# ============================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ── Qt 依赖 ─────────────────────────────────────────────────
find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Widgets
    Sql
    Network
)

# 后续模块按需开启
# WebEngineWidgets  (Phase 2 预览面板)
# Svg              (Phase 2 SVG 图标)

# ── 子模块 ───────────────────────────────────────────────────
add_subdirectory(src)
