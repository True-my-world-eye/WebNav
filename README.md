# WebNav — 桌面网页链接管理器

> 基于 C++ / Qt 6 的桌面网页链接管理器
>
> 双视图可切换 · 拖拽排序 · 链接账密存储 · Favicon 自动获取 · 可扩展云端同步

---

## 项目状态

✅ **Phase 1 核心体验完成** — 模块化构建，完整交互闭环

### 已完成的模块

| 模块 | 说明 |
|------|------|
| ✅ 项目骨架 | CMake 构建系统、分层目录结构、Git 版本管理 |
| ✅ 数据模型 | Link / Folder / Tag / LinkField 纯数据结构 |
| ✅ 数据库层 | SQLite + Repository 接口抽象 + CRUD 完整实现 |
| ✅ 核心服务 | CryptoService(DPAPI加密)、FaviconService、书签导入导出、死链检测 |
| ✅ UI 组件 | Sidebar(文件夹树+标签列表)、SearchBar、TagSelector、FieldEditor |
| ✅ 双视图 | 列表视图(Ctrl+1) + 卡片视图(Ctrl+2)，卡片自绘制含标题/域名/标签/favicon |
| ✅ 对话框 | LinkEditDialog(含附加字段+时间信息)、SettingsDialog |
| ✅ 批量操作 | 多选后右键批量打开/打标签/移动文件夹/删除 |
| ✅ 右键增强 | 复制链接、复制标题+链接 |
| ✅ 主窗口 | 工具栏(新建/编辑/打开/删除/排序)、快捷键、状态栏(共X条/选中Y条) |
| ✅ 菜单栏 | 文件(导入/导出书签)、帮助(使用说明) |
| ✅ 应用层 | Application 初始化、主题加载、系统托盘、关闭最小化到托盘 |
| ✅ 工具类 | 跨平台打开浏览器、颜色/图片工具 |

## 环境要求

| 平台 | 编译工具 | Qt 版本 |
|------|----------|---------|
| **Windows** | MinGW 11+ 或 MSVC 2022 | Qt 6.5+ (Core, Widgets, Sql, Network) |
| **macOS** | Xcode 15+ (Clang) | Qt 6.5+ for macOS |

- **CMake**: 3.20+

## 构建

### Windows 构建

```bash
# 1. 克隆仓库
git clone https://github.com/True-my-world-eye/WebNav.git
cd WebNav

# 2. CMake 配置（MinGW 示例，MSVC 同理）
cmake -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:/Qt/6.x.x/mingw_64

# 3. 编译
cmake --build build --target WebNav -- -j$(nproc)

# 4. 打包部署（复制到其他电脑可直接运行）
mkdir build/deploy
cp build/src/WebNav.exe build/deploy/
windeployqt --dir build/deploy build/deploy/WebNav.exe
```

打包后将 `build/deploy/` 整个目录复制到目标 Windows 电脑，双击 `WebNav.exe` 即可运行。

### macOS 构建

> ⚠ macOS 版本**必须**在 Mac 电脑上构建（无法从 Windows 交叉编译）。

```bash
# 1. 克隆仓库
git clone https://github.com/True-my-world-eye/WebNav.git
cd WebNav

# 2. CMake 配置
cmake -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/macos

# 3. 编译
cmake --build build --target WebNav -- -j$(sysctl -n hw.logicalcpu)

# 4. 打包为 .app 并生成 .dmg 安装包
macdeployqt build/src/WebNav.app -dmg
```

生成的 `WebNav.dmg` 可直接分发给其他 Mac 用户。

### GitHub Actions 自动化构建

本项目支持通过 GitHub Actions 在云端自动构建 Windows 和 macOS 版本，无需本地搭建环境。推送代码后，在 Actions 页面即可下载构建产物。

## 功能特色

- **双视图浏览**: 列表视图和卡片视图一键切换 (Ctrl+1 / Ctrl+2)
- **拖拽排序**: 列表视图中直接拖动行改变链接先后顺序，工具栏 ↑↓↥ 辅助排序
- **批量操作**: 多选后右键批量打开/打标签/移动文件夹/删除
- **文件夹管理**: 无限层级树形分类，右键菜单支持新建/重命名/删除
- **标签系统**: 彩色标签筛选（可点击选中/取消选中），右键可删除标签
- **Favicon 自动获取**: 异步抓取网站图标并缓存到本地，列表和卡片视图均显示
- **卡片视图**: 自绘制卡片展示标题/域名/标签/网站图标，自动适配深色/浅色主题
- **鼠标交互**:
  - 单击 → 选中
  - 双击 → 编辑链接
  - 右键 → 上下文菜单（打开/编辑/复制URL/删除 + 批量操作）
  - 拖拽 → 重新排序
- **工具栏**: ✏编辑 / 🌐打开 / 🗑删除 / ↑↓↥排序 / ⚙设置
- **菜单栏**: 文件（导入书签/导出书签）、帮助（使用说明）
- **导出格式**: 支持 CSV / HTML / Markdown 三种导出格式
- **附加字段**: 为链接保存账号/密码/邮箱/电话等凭据（密码加密存储）
- **实时搜索**: 300ms 防抖，搜索标题/URL/备注
- **深色/浅色主题**: 默认深色，设置中可切换为浅色
- **系统托盘**: 关闭窗口最小化到托盘，双击托盘恢复，右键菜单操作（显示/隐藏/退出）
- **失效链接**: 侧边栏 ⚠ 按钮一键筛选失效链接（可切换取消）
- **状态栏**: 实时显示 共X条链接 | 选中Y条
- **快捷键**: Ctrl+N 新建 / Ctrl+F 搜索 / Delete 删除

## 常用快捷键

| 快捷键 | 功能 |
|--------|------|
| `Ctrl+N` | 新建链接 |
| `Ctrl+F` | 搜索框聚焦 |
| `Ctrl+1` | 列表视图 |
| `Ctrl+2` | 卡片视图 |
| `Delete` | 删除选中链接 |
| `双击` | 编辑链接 |
| `右键` | 上下文菜单 |
| `拖拽` | 重新排序 |

> **多选操作**: 按住 Ctrl 单击选择多条，右键弹出批量操作菜单（批量打开/打标签/移动文件夹/删除）。
>
> **编辑说明**: 链接不在主页直接编辑，所有编辑操作通过双击或 ✏编辑按钮 进入对话框完成。

## 部署

将 `build/deploy/` 整个目录复制到目标电脑，双击 `WebNav.exe` 即可运行（已包含所有 Qt DLL 和插件依赖）。

## 相关文档

- [设计文档](design.md) — 完整的设计方案、数据库设计、架构说明
- [开发规范](CONTRIBUTING.md) — 代码风格、工作流程、常见陷阱
- [源码学习指南](源码学习指南.md) — 源码框架、功能说明、系统性学习路径

## 许可证

MIT

---

> 有任何问题或建议，欢迎在 GitHub Issues 反馈。
