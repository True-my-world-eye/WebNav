# WebNav — 桌面网页链接管理器

> 基于 C++ / Qt 6 的桌面网页链接管理器
> 双视图可切换 · 链接账密存储 · 可扩展云端同步

## 项目状态

✅ **Phase 1 核心体验完成** — 模块化构建，5 次提交，约 4000 行代码

### 已完成的模块

| 模块 | 说明 |
|------|------|
| ✅ 项目骨架 | CMake 构建系统、分层目录结构、Git 版本管理 |
| ✅ 数据模型 | Link / Folder / Tag / LinkField 纯数据结构 |
| ✅ 数据库层 | SQLite + Repository 接口抽象 + CRUD 完整实现 |
| ✅ 核心服务 | CryptoService(DPAPI加密)、FaviconService、书签导入导出、死链检测 |
| ✅ UI 组件 | Sidebar、SearchBar、TagSelector、FieldEditor 等 |
| ✅ 双视图 | 列表视图(Ctrl+1) + 卡片视图(Ctrl+2) |
| ✅ 对话框 | LinkEditDialog(含附加字段)、SettingsDialog、AboutDialog |
| ✅ 主窗口 | 工具栏、快捷键、状态栏、视图切换 |
| ✅ 应用层 | Application 初始化、主题加载、全局快捷键框架 |
| ✅ 工具类 | 跨平台打开浏览器、颜色/图片工具 |

## 环境要求

- **Qt**: 6.5+（需要 Core, Widgets, Sql, Network 模块）
- **CMake**: 3.20+
- **编译器**: MSVC 2022 / MinGW 11+ / GCC 11+
- **构建**: Qt WebEngine（Phase 2 预览面板需要）

## 快速开始

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt6
cmake --build .
./WebNav
```

## 项目结构

详见 [design.md](design.md) 的 §3 项目目录结构。

## 功能特色

- **双视图浏览**: 列表视图和卡片视图一键切换 (Ctrl+1 / Ctrl+2)
- **文件夹管理**: 无限层级树形分类
- **标签系统**: 彩色标签筛选
- **附加字段**: 为链接保存账号/密码/邮箱/电话等凭据（密码加密存储）
- **书签导入**: 支持 Chrome / Firefox / Edge HTML 导入
- **实时搜索**: 300ms 防抖，搜索标题/URL/备注
- **深色/浅色主题**: 跟随系统自动切换
- **快捷键**: Ctrl+N 新建 / Ctrl+F 搜索 / Ctrl+D 快速收藏
- **未来计划**: 云端同步（Phase 3, Alibaba ECS + FastAPI）

## 快捷键

| 快捷键 | 功能 |
|--------|------|
| `Ctrl+N` | 新建链接 |
| `Ctrl+F` | 搜索框聚焦 |
| `Ctrl+1` | 列表视图 |
| `Ctrl+2` | 卡片视图 |
| `Delete` | 删除选中 |
| `Enter` | 打开链接 |

## 相关文档

- [设计文档](design.md) — 完整的设计方案、数据库设计、架构说明

## 许可证

MIT
