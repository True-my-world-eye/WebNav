# WebNav 设计文档

> 基于 C++ / Qt 6 的桌面网页链接管理器
>
> 最后更新：2026-06-08

---

## 目录

1. [项目定位](#1-项目定位)
2. [功能蓝图](#2-功能蓝图)
3. [项目目录结构](#3-项目目录结构)
4. [数据库设计](#4-数据库设计)
5. [架构分层](#5-架构分层)
6. [链接附加字段系统](#6-链接附加字段系统)
7. [加密方案](#7-加密方案)
8. [双视图设计](#8-双视图设计)
9. [搜索与筛选](#9-搜索与筛选)
10. [同步架构](#10-同步架构)
11. [书签导入导出](#11-书签导入导出)
12. [UI 交互流程](#12-ui-交互流程)
13. [快捷键清单](#13-快捷键清单)
14. [主题系统](#14-主题系统)
15. [跨平台注意事项](#15-跨平台注意事项)
16. [技术栈与依赖](#16-技术栈与依赖)
17. [测试策略](#17-测试策略)

---

## 1. 项目定位

一款基于 **C++ / Qt 6 (Widgets + WebEngine)** 的桌面网页链接管理器，核心功能是帮用户保存常用网页链接并快速跳转。

**核心设计原则：**

- **操作不超过 3 步**：收藏/打开一个链接最多 2 次点击
- **渐进式复杂度**：核心功能全在界面上可见，高级功能放在右键菜单/设置中
- **数据安全**：密码类字段使用系统级加密存储
- **架构可扩展**：存储层通过接口抽象，为后续远程同步预留
- **中文注释**：所有代码的关键逻辑都附带中文说明

---

## 2. 功能蓝图

### Phase 1 — 核心体验（首版必须完成）

| 模块 | 具体内容 |
|------|----------|
| **数据持久化** | SQLite 建库，Repository 接口层 + SQLite 实现 |
| **链接管理** | 添加/编辑/删除链接，自动抓取网页标题 + favicon |
| **文件夹树** | 无限层级嵌套，拖拽排序，右键菜单（新建/重命名/删除） |
| **标签系统** | 自由打标签，彩色标签气泡显示，点击标签筛选列表 |
| **附加字段** | 预设字段[账号/密码/邮箱/电话]一键添加 + 自定义字段扩展，密码加密存储 |
| **双视图** | 列表视图（Ctrl+1）和卡片视图（Ctrl+2），工具栏按钮一键切换 |
| **搜索筛选** | 实时搜索（标题/URL/备注），按文件夹/标签/日期范围筛选 |
| **打开链接** | 双击/回车默认浏览器打开，右键复制链接 URL |
| **导入/导出** | 支持 Chrome / Firefox / Edge 书签 HTML 格式导入和导出 |
| **主题** | 浅色主题和深色主题，随系统自动切换 |
| **快捷键** | 全局快捷键支持常用操作 |

### Phase 2 — 增强体验

| 模块 | 具体内容 |
|------|----------|
| **WebEngine 预览** | 侧边/底部嵌入预览面板，选中链接显示页面快照 |
| **批量操作** | 多选后批量打开/删除/移动/打标签 |
| **全局快捷键** | 后台运行时可配置快捷键快速收藏 |
| **剪贴板监控** | 复制 URL 时托盘弹出"添加链接"提示，一键收藏 |
| **缩略图缓存** | Qt WebEngine 截图 + 本地缓存，卡片视图展示缩略图 |
| **死链检测** | 后台逐个 HEAD 请求检查，失效链接标记 + 颜色提示 |
| **多排序** | 按名称/添加时间/访问次数/最近访问排序，正序倒序可切换 |
| **链接笔记** | 每条链接可加富文本备注/摘录 |
| **QR 码分享** | 右键链接生成 QR 码，手机扫码打开 |
| **工作区** | 保存一组链接为工作区，一键全部打开 |

### Phase 3 — 高级功能

| 模块 | 具体内容 |
|------|----------|
| **云同步** | 通过自建服务器（Alibaba ECS）同步多设备数据 |
| **稍后阅读** | 阅读队列 + 已读/未读标记 + 自动归档 |
| **定时提醒** | 对链接设置提醒时间，到点系统通知 |
| **统计面板** | 总链接数、失效数、访问排行、标签分布可视化 |
| **Markdown 导出** | 将收藏夹导出为优雅的 Markdown 文档 |

---

## 3. 项目目录结构

```
WebNav/
├── CMakeLists.txt                      # 顶层构建文件
├── README.md                           # 使用说明
├── design.md                           # ★ 本文件：设计文档
├── .clang-format
├── .gitignore
│
├── src/
│   ├── main.cpp                        # 应用入口
│   │
│   ├── app/                            # 应用层 — 全局生命周期与系统集成
│   │   ├── CMakeLists.txt
│   │   ├── Application.h/.cpp          # 初始化、主题切换、系统托盘
│   │   └── GlobalHotkey.h/.cpp         # 全局快捷键注册
│   │
│   ├── core/                           # 核心业务层 — 纯逻辑，无 Qt Widgets 依赖
│   │   ├── CMakeLists.txt
│   │   ├── database/                   # 数据持久化
│   │   │   ├── DatabaseManager.h/.cpp      # SQLite 初始化与版本迁移
│   │   │   ├── interfaces/                # ★ 仓库接口（为未来远程实现预留）
│   │   │   │   ├── ILinkRepository.h
│   │   │   │   ├── IFolderRepository.h
│   │   │   │   └── ITagRepository.h
│   │   │   └── impl/
│   │   │       ├── SqliteLinkRepository.h/.cpp
│   │   │       ├── SqliteFolderRepository.h/.cpp
│   │   │       └── SqliteTagRepository.h/.cpp
│   │   ├── models/                     # 数据模型（纯 struct）
│   │   │   ├── Link.h
│   │   │   ├── Folder.h
│   │   │   ├── Tag.h
│   │   │   └── LinkField.h             # ★ 附加字段模型
│   │   └── services/                   # 业务服务
│   │       ├── FaviconService.h/.cpp       # favicon 异步抓取与本地缓存
│   │       ├── LinkCheckService.h/.cpp     # 后台死链检测
│   │       ├── ClipboardService.h/.cpp     # 剪贴板 URL 监听
│   │       ├── BookmarkImporter.h/.cpp     # Chrome/Firefox HTML 导入解析
│   │       ├── BookmarkExporter.h/.cpp     # HTML / Markdown 导出
│   │       ├── ThumbnailService.h/.cpp     # 网页截图缩略图
│   │       └── CryptoService.h/.cpp        # ★ 密码加密/解密（DPAPI）
│   │
│   ├── ui/                            # UI 层 — 所有界面组件
│   │   ├── CMakeLists.txt
│   │   ├── MainWindow.h/.cpp          # 主窗口框架（布局编排）
│   │   ├── widgets/                   # 可复用 UI 组件
│   │   │   ├── Sidebar.h/.cpp             # 左侧导航栏
│   │   │   ├── SearchBar.h/.cpp           # 搜索输入框
│   │   │   ├── TagSelector.h/.cpp         # 标签选择/管理控件
│   │   │   ├── TagLabel.h/.cpp            # 标签彩色气泡
│   │   │   ├── FieldEditor.h/.cpp         # ★ 附加字段编辑器
│   │   │   └── LinkPreviewPanel.h/.cpp    # WebEngine 预览面板
│   │   ├── views/                     # 数据展示视图
│   │   │   ├── LinkListView.h/.cpp        # 列表视图（QTableView）
│   │   │   ├── LinkCardView.h/.cpp        # 卡片视图（QListWidget + 自定义绘制）
│   │   │   ├── LinkDelegate.h/.cpp        # 列表/卡片自定义代理
│   │   │   └── FolderTreeView.h/.cpp      # 文件夹树
│   │   └── dialogs/                   # 弹窗对话框
│   │       ├── LinkEditDialog.h/.cpp      # 添加/编辑链接（含附加字段）
│   │       ├── SettingsDialog.h/.cpp      # 偏好设置
│   │       ├── SyncConfigDialog.h/.cpp    # 同步配置
│   │       ├── AboutDialog.h/.cpp
│   │       └── QRCodeDialog.h/.cpp
│   │
│   └── utils/                        # 工具类
│       ├── CMakeLists.txt
│       ├── PlatformUtils.h/.cpp       # 打开浏览器、系统通知封装
│       ├── ColorUtils.h/.cpp          # 标签颜色生成/转换
│       └── ImageUtils.h/.cpp          # 图片缩放、缓存路径管理
│
├── resources/
│   ├── resources.qrc                  # Qt 资源索引
│   ├── icons/
│   │   ├── app/                       # 应用图标（多尺寸 .ico / .png）
│   │   └── actions/                   # 工具栏/菜单 SVG 图标
│   ├── themes/
│   │   ├── light.qss                  # 浅色主题样式表
│   │   └── dark.qss                   # 深色主题样式表
│   └── database/
│       └── schema.sql                 # 初始建表 SQL
│
├── sync-server/                       # ★ 服务端同步组件（Phase 3）
│   ├── README.md
│   ├── docker-compose.yml
│   ├── Dockerfile
│   ├── requirements.txt
│   ├── app/
│   │   ├── main.py                    # FastAPI 入口
│   │   ├── models.py
│   │   ├── routes/
│   │   │   ├── sync.py                # 同步接口
│   │   │   └── auth.py                # Token 认证
│   │   └── database.py
│   └── nginx/
│       └── webnav-sync.conf           # Nginx 反向代理配置样例
│
├── tests/
│   ├── CMakeLists.txt
│   ├── unit/
│   │   ├── test_database_manager.cpp
│   │   ├── test_link_repository.cpp
│   │   ├── test_folder_repository.cpp
│   │   ├── test_tag_repository.cpp
│   │   ├── test_bookmark_importer.cpp
│   │   └── test_crypto_service.cpp
│   └── integration/
│       └── test_favicon_service.cpp
│
└── third_party/
    └── libqrencode/                  # QR 码生成库（纯 C，源码包含）
```

---

## 4. 数据库设计

```sql
-- ============================================================
-- 文件夹表
-- ============================================================
CREATE TABLE folders (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT NOT NULL,                              -- 文件夹名称
    parent_id   INTEGER REFERENCES folders(id) ON DELETE CASCADE,  -- 父文件夹 ID（NULL=根节点）
    sort_order  INTEGER NOT NULL DEFAULT 0,                 -- 同级排序序号
    created_at  TEXT NOT NULL DEFAULT (datetime("now")),
    updated_at  TEXT NOT NULL DEFAULT (datetime("now"))
);

-- ============================================================
-- 标签表
-- ============================================================
CREATE TABLE tags (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT NOT NULL UNIQUE,                       -- 标签名称（唯一）
    color       TEXT NOT NULL DEFAULT "#5B9BD5",            -- 标签颜色（十六进制）
    created_at  TEXT NOT NULL DEFAULT (datetime("now"))
);

-- ============================================================
-- 链接表（核心表）
-- ============================================================
CREATE TABLE links (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    folder_id       INTEGER REFERENCES folders(id) ON DELETE SET NULL,
    title           TEXT NOT NULL DEFAULT "",
    url             TEXT NOT NULL,
    description     TEXT NOT NULL DEFAULT "",
    notes           TEXT NOT NULL DEFAULT "",
    favicon_path    TEXT,
    thumbnail_path  TEXT,
    visit_count     INTEGER NOT NULL DEFAULT 0,
    last_visited_at TEXT,
    is_broken       INTEGER NOT NULL DEFAULT 0,
    sort_order      INTEGER NOT NULL DEFAULT 0,             -- 拖拽排序序号
    created_at      TEXT NOT NULL DEFAULT (datetime("now")),
    updated_at      TEXT NOT NULL DEFAULT (datetime("now")),
    -- 以下为同步预留字段
    sync_version    INTEGER NOT NULL DEFAULT 1,             -- 同步版本号（冲突检测用）
    sync_updated_at TEXT
);

-- ============================================================
-- 链接-标签关联表（多对多）
-- ============================================================
CREATE TABLE link_tags (
    link_id INTEGER NOT NULL REFERENCES links(id) ON DELETE CASCADE,
    tag_id  INTEGER NOT NULL REFERENCES tags(id) ON DELETE CASCADE,
    PRIMARY KEY (link_id, tag_id)
);

-- ============================================================
-- ★ 链接附加字段表（账号/密码/邮箱/电话/自定义）
-- ============================================================
CREATE TABLE link_fields (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    link_id      INTEGER NOT NULL REFERENCES links(id) ON DELETE CASCADE,
    field_key    TEXT NOT NULL,
    field_value  TEXT NOT NULL DEFAULT "",
    field_type   INTEGER NOT NULL DEFAULT 0,
                 -- 0=明文文本  1=加密存储  2=多行文本
    is_password  INTEGER NOT NULL DEFAULT 0,   -- 1=密码类型（界面用 ● 遮盖）
    sort_order   INTEGER NOT NULL DEFAULT 0,
    created_at   TEXT NOT NULL DEFAULT (datetime("now"))
);

CREATE INDEX idx_links_folder ON links(folder_id);
CREATE INDEX idx_links_url ON links(url);
CREATE INDEX idx_links_created ON links(created_at);
CREATE INDEX idx_link_fields_link ON link_fields(link_id);
```

---

## 5. 架构分层

```
┌────────────────────────────────────────────┐
│  app/       应用层                          │
│  Application, GlobalHotkey                 │
│  负责：初始化、主题切换、系统托盘、全局热键    │
├────────────────────────────────────────────┤
│  ui/        界面层                          │
│  MainWindow, Widgets, Views, Dialogs       │
│  负责：用户交互、数据显示、布局编排            │
│  依赖：core/ 中的 Service 和 Repository      │
├────────────────────────────────────────────┤
│  core/      核心业务层（无 UI 依赖）          │
│  ├─ services/    业务服务                   │
│  ├─ database/    数据访问                   │
│  │  ├─ interfaces/  ★ 纯虚接口              │
│  │  └─ impl/        SQLite 实现             │
│  └─ models/      纯数据结构                  │
├────────────────────────────────────────────┤
│  utils/      工具层                         │
│  PlatformUtils, ColorUtils, ImageUtils     │
└────────────────────────────────────────────┘
```

**关键约束：** core/ 层不得包含任何 Qt Widgets 头文件，只能引用 Qt Core 和 Qt Sql。

---

## 6. 链接附加字段系统（关键设计）

### 6.1 设计目标

允许用户为每条链接存储登录凭据等附加信息。支持预设字段一键添加，也支持任意自定义字段扩展。

**⚠ 重要约束：所有字段均为可选填，不填任何字段也可正常保存链接。**

### 6.2 预设字段

| 预设字段 | field_key  | 说明 |
|----------|------------|------|
| 账号     | account    | 登录账号/用户名 |
| 密码     | password   | 登录密码（加密存储，界面 ● 遮盖） |
| 邮箱     | email      | 关联邮箱地址 |
| 电话     | phone      | 关联电话号码 |

界面提供四个预设字段的快捷添加按钮，点击即添加字段行到当前链接。

### 6.3 自定义字段

用户可通过交互添加任意名称的字段，例如：API Token、服务器地址、邀请码等。

### 6.4 UI 交互

```
┌─ 编辑链接 ────────────────────────────────────┐
│ URL:    https://example.com/login             │
│ 标题:   示例网站                               │
│ 文件夹: 工作                                   │
│ 标签:   [登录] [+添加标签]                     │
│ 备注:   后端管理平台                           │
│                                                │
│ ▼ 附加数据                                    │
│  ┌─────────────────────────────────────────┐  │
│  │ [+账号] [+密码] [+邮箱] [+电话]         │  │
│  │ 账号: admin                     [×]     │  │
│  │ 密码: ·········           [👁] [×]     │  │
│  │ 邮箱: admin@ex.com           [×]        │  │
│  │ [+ 添加自定义字段]                       │  │
│  └─────────────────────────────────────────┘  │
│                              [取消]  [保存]     │
└────────────────────────────────────────────────┘
```

**交互要点：**

- 点击预设按钮（如 [+账号]）→ 添加对应字段行
- 点击字段行右侧 [×] → 移除该字段
- 密码字段默认 ● 遮盖，点击 [👁] 切换明文显示
- 点击 [+ 添加自定义字段] → 弹出输入框，输入字段名和值，可勾选"加密存储"
- **全部字段均为可选，没有必填校验**

---

## 7. 加密方案

| 平台 | 方式 | 说明 |
|------|------|------|
| Windows | DPAPI (CryptProtectData) | 绑定当前 Windows 用户凭据，无需额外密码 |
| macOS | Keychain Services | 系统钥匙串 |
| Linux | libsecret / AES-256-GCM | 优先 libsecret，不可用时 fallback |

抽象为 CryptoService 接口，各平台实现不同后端。Phase 1 仅实现 Windows DPAPI。

---

## 8. 双视图设计

- **列表视图（Ctrl+1）**：QTableView + 自定义代理，列：favicon / 标题 / URL / 文件夹 / 标签 / 日期
- **卡片视图（Ctrl+2）**：QListWidget + 自定义代理，卡片：缩略图 + 标题 + 域名 + 标签气泡
- 工具栏按钮切换，偏好持久化

---

## 9. 搜索与筛选

- **搜索框**：顶部工具栏，300ms 防抖实时搜索标题/URL/备注
- **文件夹筛选**：点击侧边栏树节点
- **标签筛选**：点击标签气泡
- **日期筛选**：日期选择器
- **组合筛选**：以上条件可叠加
- **智能列表**：所有链接 / 最近添加 / 频繁访问 / 失效链接

---

## 10. 同步架构（Phase 3）

### 10.1 接口抽象（Phase 1 完成）

```cpp
class ILinkRepository {
public:
    virtual ~ILinkRepository() = default;
    virtual QVector<Link> getAll() = 0;
    virtual std::optional<Link> getById(int id) = 0;
    virtual int insert(const Link& link) = 0;
    virtual bool update(const Link& link) = 0;
    virtual bool remove(int id) = 0;
    virtual QVector<Link> getChangedSince(const QDateTime& since) = 0;  // 同步用
};
```

### 10.2 部署架构

```
桌面应用 ── HTTPS ──→ Nginx（服务器, 不影响 Hexo 博客）
                       /api/webnav/* → proxy_pass localhost:8123
                       sync-server (FastAPI + SQLite, Token 鉴权)
```

### 10.3 同步时序

1. 启动时拉取远程增量数据
2. 按 updated_at 合并冲突（后写入者获胜）
3. 推送本地变更到服务端
4. 密码字段加密后传输，服务端无法解密

---

## 11. 书签导入导出

- 支持 Chrome / Firefox / Edge 的 Netscape HTML 书签格式
- 导入时创建对应文件夹层级，按 URL 去重
- 导出为标准 HTML 书签文件

---

## 12. UI 交互流程

### 主窗口布局

```
┌────────────────────────────────────────────────────────┐
│  [🔍 搜索...]     [+新建]  [列表/卡片切换]  [⚙ 设置]   │
│  ┌──────┬───────────────────────────────────────────┐ │
│  │侧边栏 │  链接列表/卡片区域                          │ │
│  │ 所有  │                                           │ │
│  │ 最近  │  （列表模式）                              │ │
│  │ 频繁  │  🌐 标题1        url...    2026-06-06     │ │
│  │ ───── │  🌐 标题2        url...    2026-06-05     │ │
│  │ 工作  │                                           │ │
│  │ 学习  │  或（卡片模式）                            │ │
│  │ ───── │  ┌─────┐ ┌─────┐ ┌─────┐                │ │
│  │ #标签  │  │ 截图 │ │ 截图 │ │ 截图 │              │ │
│  │ #前端  │  │标题1 │ │标题2 │ │标题3 │              │ │
│  └──────┴───────────────────────────────────────────┘ │
│  状态栏: 共 128 条 | 选中 3 条                        │
└────────────────────────────────────────────────────────┘
```

### 关键交互规则

| 触发方式 | 列表视图 | 卡片视图 |
|---------|----------|----------|
| 单击 | 选中行 | 选中卡片 |
| 双击 | 编辑链接 | 编辑链接 |
| 右键 | 上下文菜单（打开/编辑/删除） | 上下文菜单（打开/编辑/删除） |
| Delete键 | 删除选中 | 删除选中 |
| 拖拽行 | 重新排序 | — |

**重要**: 列表和卡片视图均只读（NoEditTriggers），所有修改通过编辑对话框完成。

### 添加链接流程

1. Ctrl+N 或 [+新建] → 弹出 LinkEditDialog
2. 粘贴 URL → 自动抓取标题 + favicon
3. 选文件夹 + 打标签（可选）
4. 展开"附加数据"，按需添加字段（全部可选）
5. 保存

### 编辑链接流程

1. 双击列表项（或选中后点✏编辑按钮）→ 弹出已填充的 LinkEditDialog
2. 修改 URL / 标题 / 文件夹 / 标签 / 备注 / 附加字段
3. 对话框底部显示创建时间和最后更新时间
4. 保存后自动更新列表

### 打开链接流程

1. 选中一条链接
2. 点击工具栏🌐"打开"按钮（或右键 → 打开链接）
3. 系统默认浏览器自动打开该链接
4. 访问次数自动 +1，最近访问时间更新

---

## 13. 快捷键清单

| 快捷键 | 功能 |
|--------|------|
| Ctrl+N | 新建链接 |
| Ctrl+F | 搜索框聚焦 |
| Ctrl+1 | 列表视图 |
| Ctrl+2 | 卡片视图 |
| Delete | 删除选中链接 |
| 双击 | 编辑链接 |
| 右键 | 上下文菜单（打开/编辑/删除）|
| 拖拽 | 重新排序（列表视图）|
| Enter | 打开选中链接（通过工具栏🌐按钮）|
| Escape | 关闭弹窗/清空搜索 |

---

## 14. 主题系统

- QSS 样式表，light.qss / dark.qss 两套
- 启动时跟随系统，设置中可手动切换
- 切换即时生效，无需重启

---

## 15. 跨平台注意事项

| 模块 | 需适配内容 |
|------|-----------|
| 加密 | Windows DPAPI / macOS Keychain / Linux libsecret |
| 全局快捷键 | Win RegisterHotKey / macOS CGEvent / Linux XCB |
| 路径 | 统一用 QStandardPaths |
| 系统托盘 | QSystemTrayIcon 跨平台 |
| 打开浏览器 | QDesktopServices::openUrl 跨平台 |

---

## 16. 技术栈与依赖

- **语言**: C++17
- **构建**: CMake 3.20+
- **Qt**: 6.5+ (Core, Widgets, Sql, Network, WebEngineWidgets)
- **数据库**: SQLite（QtSql 内置）
- **加密**: Windows DPAPI（Win32 API）
- **二维码**: libqrencode（third_party 源码包含，Phase 2）
- **服务端**: Python FastAPI（Phase 3）

---

## 17. 测试策略

- **单元测试**: DatabaseManager、Repository、Models、BookmarkParser、CryptoService
- **集成测试**: 完整 CRUD 流程、导入导出往返验证
## 18. 开发进度总览

### Phase 1 进度

| 模块 | 状态 | 提交信息 | 说明 |
|------|------|----------|------|
| Module 1: 项目骨架 | ✅ 完成 | `e823532` | CMake 构建系统、目录结构、.gitignore、README、design.md |
| Module 2: 数据模型 | ✅ 完成 | `f9bd77f` | Link / Folder / Tag / LinkField 纯数据结构 |
| Module 3: 数据库层 | ✅ 完成 | `2e247d2` | DatabaseManager、Repository 接口 + SQLite CRUD |
| Module 4: 核心服务 | ✅ 完成 | `4f10fc5` | CryptoService DPAPI、FaviconService、书签导入导出、死链检测 |
| Module 5: UI 组件 | ✅ 完成 | `5c92cc8` | Sidebar、SearchBar、TagSelector、TagLabel、FieldEditor |
| Module 6: UI 双视图 | ✅ 完成 | `5c92cc8` | LinkListView、LinkCardView、LinkDelegate、FolderTreeView |
| Module 7: UI 对话框 | ✅ 完成 | `5c92cc8` | LinkEditDialog、SettingsDialog、AboutDialog 等 |
| Module 8: 主窗口+应用 | ✅ 完成 | `5c92cc8` | MainWindow、Application、工具栏、快捷键 |
| Module 9: 工具类 | ✅ 完成 | `5c92cc8` | PlatformUtils、ColorUtils、ImageUtils |

### 代码统计

- **头文件**: 40 个 (.h) + **源文件**: 33 个 (.cpp)
- **提交次数**: 5 次 | **总代码量**: ~4000 行

### 下一步开发指引

1. 实现批量操作功能（批量打开/删除/移动/打标签）
2. 添加导入导出对话框的 UI 入口（当前仅后端支持）
3. 实现死链检测的 UI 显示和交互
4. 完善卡片视图的拖拽排序
5. 开始 Phase 2 功能（WebEngine 预览面板、缩略图缓存等）

---

## 19. Bug 修复记录

> 本节记录开发/测试中遇到的运行时 Bug 及其修复方案，便于后续回溯。

### Bug 1: schema.sql 建表语句因前置注释被跳过

**症状**：启动日志显示 CREATE INDEX 因"no such table"失败，所有表未创建，插入报"Parameter count mismatch"。

**根因**：DatabaseManager::executeSchema() 将 schema.sql 按 ; 分割后，用 	rimmed.startsWith("--") 判断是否为注释行。但每个 CREATE TABLE 前都有 -- 文件夹表 这样的注释行，分割后的语句以 -- 开头，整条被跳过。只有不带前置注释的 CREATE INDEX 被执行，但因表不存在而失败。

**修复**：将单行 startsWith("--") 判断改为逐行过滤——只移除纯注释行，保留后续 SQL 再执行。

**涉及文件**：
- src/core/database/DatabaseManager.cpp — executeSchema() 方法

**修复提交**：—
### Bug 2: 默认构造的 QString 被 Qt SQLite 驱动绑定为 SQL NULL

**症状**：建表成功后，插入链接报 NOT NULL constraint failed: links.description。

**根因**：C++ 中 QString() 默认构造的是 **null string**（isNull() == true，与 QString("") 不同——后者是非 null 的空字符串）。当 QVariant(QString()) 传入 QSqlQuery::addBindValue() 时，Qt SQLite 驱动检查 QVariant 内部值的 isNull 状态，为 null 则绑定为 SQLite NULL。而 links.description 列定义为 TEXT NOT NULL DEFAULT ""，无法接受 NULL。

**影响范围**：所有通过 ddBindValue 传入默认构造 QString 的 NOT NULL TEXT 列（insert 和 update 方法均受影响）。

**修复**：在 SqliteLinkRepository::insert() 和 update() 中，对 	itle、description、
otes、aviconPath、	humbnailPath 五个字段用 link.field.isNull() ? QString("") : link.field 显式将 null QString 转为空字符串后再绑定。url 为必填字段，保持原样。

**涉及文件**：
- src/core/database/impl/SqliteLinkRepository.cpp — insert() 和 update() 方法

### Bug 3: 链接表缺少 sort_order 列导致无法拖拽排序
**症状**：新增"拖拽改变链接顺序"功能后，发现 `links` 表缺少排序字段，无法持久化用户调整的顺序。
**根因**：原 `links` 表没有 `sort_order` 列，`ORDER BY` 只能按时间排序，无法记录用户手动调整的顺序。
**修复**：
- `schema.sql` 中 `links` 表新增 `sort_order INTEGER NOT NULL DEFAULT 0` 列
- `Link.h` 新增 `sortOrder = 0` 字段
- `SqliteLinkRepository` 新增 `reorderLinks()` 方法，批量更新排序
- `DatabaseManager::migrateIfNeeded()` 新增迁移逻辑，对旧数据库执行 `ALTER TABLE` 添加该列
- 列表视图启用 `InternalMove` 拖拽模式，拖拽完成后通过 `onLinksReordered()` 更新数据库
**涉及文件**：
- `resources/database/schema.sql`
- `src/core/models/Link.h`
- `src/core/database/interfaces/ILinkRepository.h`
- `src/core/database/impl/SqliteLinkRepository.h/.cpp`
- `src/core/database/DatabaseManager.cpp`
- `src/ui/MainWindow.cpp`

**修复提交**：—

### Bug 4: 标签设置后无法保留/显示（缺失可视反馈）

**症状**：在编辑对话框中为链接添加标签，关闭后标签未绑定到链接，主界面不显示标签。

**根因**：`TagSelector` 只有输入框，没有已选标签的可视反馈区，用户看不到已选标签。且 `QCompleter` 弹出时回车可能被拦截导致 `returnPressed` 不触发。

**修复**：
- 新增 `FlowLayout` 流式布局，已选标签以圆角气泡 (chip) 形式显示，支持自动折行
- 每个 chip 可点击移除
- 双重选中途径：`returnPressed` + `QCompleter::activated`
- 新增标签即时通过 `addSelectedTagId()` 加入选中集

**涉及文件**：
- `src/ui/widgets/TagSelector.h/.cpp`

### Bug 5: 主题切换无法保存 / 默认跟随系统不可用

**症状**：设置中切换主题后，重启软件回到浅色主题。

**根因**：
1. `QSettings` 未正确保存和读取主题值
2. `Application::applyTheme()` 路径拼接 `":/themes/system.qss"` 不存在
3. QSS 文件（`light.qss` / `dark.qss`）内容为空

**修复**：
- 删除 "随系统" 选项，默认深色
- `QSettings` 读写 `"theme"` 键
- `Application` 启动时从 `QSettings` 加载主题
- 编写完整的浅色/深色 QSS 主题样式表（~280 行）

**涉及文件**：
- `src/app/Application.cpp`
- `src/ui/dialogs/SettingsDialog.h/.cpp`
- `resources/themes/light.qss` / `dark.qss`

### Bug 6: 拖拽排序导致行消失/覆盖

**症状**：列表视图拖拽行后，行数据消失或覆盖其他行。

**根因**：`QTableView` + `InternalMove` + `setSortingEnabled(true)` 冲突。Qt 内置排序和拖拽同时启用时，行移动后内部排序立即介入，导致混乱。

**修复**：
- 禁用 `setSortingEnabled(false)`
- 覆写 `LinkListView::dropEvent()`，在拖拽完成后读取 model 新顺序
- 通过 `linkDropped(fromRow, toRow)` 信号通知 MainWindow 持久化排序并刷新页面

**涉及文件**：
- `src/ui/views/LinkListView.h/.cpp`
- `src/ui/MainWindow.cpp`

### Bug 7: Windows 编译后带控制台窗口

**症状**：双击 WebNav.exe 启动时，会同时弹出一个命令行窗口。

**根因**：`CMakeLists.txt` 中 `WIN32_EXECUTABLE` 属性被 `AND NOT MINGW` 条件限制（针对 MSVC），MinGW 编译时缺少该属性。

**修复**：移除 `AND NOT MINGW`，统一启用 `WIN32_EXECUTABLE`。

**涉及文件**：
- `src/CMakeLists.txt`

### Bug 8: 侧边栏 + 按钮显示异常（形如复选框）

**症状**：侧边栏标题旁的 "+" 按钮显示为复选框样式。

**根因**：全局 `QPushButton` QSS 样式有 `padding: 6px 16px` 和 `min-height`，使 20×20 的小按钮被拉伸变形。

**修复**：
- 添加 `#sidebarAddBtn` 专用样式，固定 `min/max-width/height: 20px`、`padding: 0px`
- 在浅色/深色 QSS 中分别添加该对象名的样式定义

**涉及文件**：
- `src/ui/widgets/Sidebar.cpp`（添加 `setObjectName("sidebarAddBtn")`）
- `resources/themes/light.qss` / `dark.qss`