# WebNav 开发规范

> 本文档记录 WebNav 项目的开发约定、代码风格、工作流程和注意事项。
> 开发时请参照本文档，保持代码风格一致，提升团队协作效率。

---

## 目录

1. [代码风格](#1-代码风格)
2. [目录与文件命名](#2-目录与文件命名)
3. [C++ 编码约定](#3-c-编码约定)
4. [Qt 使用规范](#4-qt-使用规范)
5. [数据库规范](#5-数据库规范)
6. [UI 开发规范](#6-ui-开发规范)
7. [Git 工作流](#7-git-工作流)
8. [构建与测试](#8-构建与测试)
9. [常见陷阱](#9-常见陷阱)

---

## 1. 代码风格

### 1.0 工作习惯

- **复杂任务创建待办清单**：处理跨多文件的复杂任务前，先创建 TaskCreate 待办事项清单，跟踪进度并确保按时完成。
- **先阅读后修改**：修改文件前先完整阅读其内容，确认理解现有逻辑后再动手。

### 1.1 通用原则

- **可读性优先**：代码是写给人读的，适当注释胜过魔法数字。
- **防御性编程**：假设输入可能非法，对边界情况做保护。
- **最小改动原则**：修复 bug 时只改必要的地方，不顺便重构无关代码。
- **中文注释**：关键逻辑使用中文注释，便于团队理解。

### 1.2 命名规范

| 类别 | 规范 | 示例 |
|------|------|------|
| 类名 | PascalCase | `DatabaseManager`, `SqliteLinkRepository` |
| 函数/方法 | camelCase | `refreshLinks()`, `getById()` |
| 成员变量 | m_前缀 + camelCase | `m_linkRepo`, `m_currentLink` |
| 局部变量 | camelCase | `linkId`, `folderNames` |
| 常量/枚举 | PascalCase | `ViewMode::ListView` |
| 文件命名 | PascalCase + .cpp/.h | `LinkEditDialog.cpp` |
| 接口类 | I前缀 + PascalCase | `ILinkRepository` |

### 1.3 文件组织

- .h 和 .cpp 成对出现，同名同目录
- 头文件使用 `#pragma once`
- 每个头文件只声明一个主要类
- 头文件包含顺序：自身头文件 -> Qt 标准库 -> 项目内头文件

---

## 2. 目录与文件命名

### 2.1 目录结构规范

```
src/
  core/         # 核心业务层（无 UI 依赖）
    database/   # 数据持久化
    models/     # 纯数据结构
    services/   # 业务服务
  ui/           # 界面层
    widgets/    # 可复用 UI 组件
    views/      # 数据展示视图
    dialogs/    # 弹窗对话框
  utils/        # 工具类
  app/          # 应用层生命周期
```

### 2.2 文件命名规则

- 源文件与头文件同名：`DatabaseManager.h` / `DatabaseManager.cpp`
- 接口头文件：`ILinkRepository.h`
- 命名意义清晰，不缩写：`FolderTreeView` 不是 `FldTreeVw`

---

## 3. C++ 编码约定

### 3.1 语言标准

- C++17，禁止使用 C++20 特性
- 使用 `std::optional` 替代指针返回值表示"可能无值"
- 使用 `auto` 简化类型声明，但避免过度使用

### 3.2 智能指针

- 优先使用 `std::unique_ptr` 表示独占所有权
- UI 对象使用 Qt 父子对象机制（`parent` 参数），不手动 delete
- 避免裸 `new`/`delete`，除非在 Qt 对象构造中

### 3.3 错误处理

- 数据库操作失败时使用 `qWarning()` 输出错误信息
- UI 操作失败时使用 `statusBar()->showMessage()` 提示用户
- 不抛出异常（Qt 默认禁用异常）

### 3.4 字符串处理

- **重要：QString() 默认构造为 null string（isNull() == true），与 QString("") 不同**
- 需要空字符串时显式使用 `QString("")` 或 `QStringLiteral("")`
- 传递给 `QSqlQuery::addBindValue` 的 QString 若无值，应显式转为空字符串：
  ```cpp
  query.addBindValue(link.field.isNull() ? QString("") : link.field);
  ```
- 使用 `QStringLiteral` 包装字面量（编译时优化）

---

## 4. Qt 使用规范

### 4.1 信号与槽

- 使用新式连接语法：`connect(sender, &Class::signal, receiver, &Class::slot)`
- Lambda 表达式中的捕获尽量明确：`[this]` 而非 `[=]`
- 自定义信号：声明在 `signals:` 区域，返回 void

### 4.2 UI 开发

- 核心业务逻辑放在 `core/` 层，不依赖 Qt Widgets
- UI 组件只负责展示和交互，不直接操作数据库
- 仓库接口（`ILinkRepository` 等）通过构造函数依赖注入
- 列表项不可直接编辑：设置 `NoEditTriggers`，通过独立编辑按钮修改
- 双击事件默认打开编辑对话框，通过独立"打开"按钮跳转浏览器

### 4.3 资源管理

- 静态资源（图标、样式表、SQL 文件）放在 `resources/` 目录
- 使用 `.qrc` 文件注册资源
- SVG 是首选的图标格式（Qt 原生支持，无分辨率问题）

### 4.4 界面交互规范（重要）

| 操作 | 行为 |
|------|------|
| 双击列表项 | 打开编辑对话框 |
| 右键列表项 | 弹出上下文菜单（打开/编辑/删除） |
| 工具栏编辑按钮 | 编辑当前选中项 |
| 工具栏打开按钮 | 在系统浏览器中打开链接 |
| 工具栏删除按钮 | 删除当前选中项（带确认提示） |
| Delete 键 | 删除当前选中项 |
| Ctrl+N | 新建链接 |
| Ctrl+F | 搜索框聚焦 |

---

## 5. 数据库规范

### 5.1 Schema 变更

- 所有建表语句写在 `resources/database/schema.sql`
- 版本迁移逻辑在 `DatabaseManager::migrateIfNeeded()` 中实现
- 不要手动修改数据库文件

### 5.2 SQL 语句

- 列名使用 snake_case：`folder_id`, `created_at`
- TEXT 类型列存储日期时间，使用 ISO 8601 格式：`datetime("now")`
- `NOT NULL` 列必须有 `DEFAULT` 值

### 5.3 关于 schema.sql 的执行

`schema.sql` 中的 SQL 语句通过 `DatabaseManager::executeSchema()` 按分号分割后逐条执行。**注意**：该实现会过滤掉以 `--` 开头的行，但保留有效 SQL。因此：

- 多行 SQL 前的注释行会被自动移除
- 不要用注释行"包围"整条语句
- **新增表时确保 `CREATE TABLE` 本身不是以 `--` 开头**

---

## 6. UI 开发规范

### 6.1 组件职责

| 组件 | 职责 |
|------|------|
| MainWindow | 主窗口框架、工具栏、信号编排、业务调度 |
| Sidebar | 文件夹树 + 快捷筛选（所有链接/最近/频繁） |
| SearchBar | 搜索输入，300ms 防抖 |
| LinkListView | 表格形式展示链接列表 |
| LinkCardView | 卡片网格展示链接 |
| LinkDelegate | 列表/卡片自定义绘制代理 |
| LinkEditDialog | 新建/编辑链接表单 |
| FieldEditor | 附加字段（账号/密码/邮箱等）编辑 |
| TagSelector | 标签选择/新建 |

### 6.2 交互设计原则

- 列表/卡片视图中的内容不可直接编辑（`NoEditTriggers`）
- 所有修改操作通过独立的编辑按钮或双击触发编辑对话框
- 打开链接必须通过显式的"打开"按钮或右键菜单
- 删除操作必须有确认对话框
- 操作结果通过状态栏反馈

### 6.3 视图交互矩阵

| 触发方式 | 列表视图 | 卡片视图 |
|---------|----------|----------|
| 单击 | 选中行 | 选中卡片 |
| 双击 | 编辑链接 | 编辑链接 |
| 右键 | 上下文菜单 | 上下文菜单 |
| Delete键 | 删除选中 | 删除选中 |

---

## 7. Git 工作流

### 7.1 分支策略

- `main` 分支保持稳定可构建
- 功能开发在 `codex/feature-name` 分支进行（prefix: `codex/`）
- 修复 bug 在 `codex/fix-bug-name` 分支进行

### 7.2 提交信息规范

```
<type>: <简短的变更说明>

<详细描述（可选）>
```

类型前缀：
- `feat:` 新功能
- `fix:` 修复 bug
- `docs:` 文档更新
- `refactor:` 重构
- `style:` 代码格式
- `chore:` 构建/工具链

### 7.3 提交前检查

1. 代码能编译通过
2. 只提交必要文件（排除 build/ 产物、数据库文件）
3. 提交信息清晰说明变更内容
4. 代码风格符合本规范

---

## 8. 构建与测试

### 8.1 构建方式

```bash
# 配置
cmake -B build -G Ninja

# 构建
cmake --build build --target WebNav -- -j8

# 运行
./build/src/WebNav.exe
```

### 8.2 运行前

首次运行或数据库 schema 变更后，建议删除旧数据库文件以确保干净初始化：

```bash
rm "$env:APPDATA/WebNav/WebNav/webnav.db"
```

数据库路径由 `QStandardPaths::AppDataLocation` 决定：
- Windows: `C:/Users/<user>/AppData/Roaming/WebNav/WebNav/webnav.db`

---

## 9. 常见陷阱

### 9.1 Bug 修复记录

详见 [design.md §18 Bug 修复记录](design.md)。

### 9.2 已知陷阱速查

| 陷阱 | 说明 | 修复方式 |
|------|------|----------|
| **Null QString 绑定为 SQL NULL** | `QString()` 默认构造为 null，`addBindValue` 将其绑为 SQL NULL | 使用 `.isNull() ? QString("") : value` 保护 |
| **注释行跳过建表语句** | `executeSchema` 按 `;` 分割后检查 `startsWith("--")`，前置注释导致整条语句被跳过 | 下一版本改为逐行过滤注释 |
| **schema.sql 变更后未清理旧 DB** | 旧数据库结构不匹配，导致 `CREATE TABLE IF NOT EXISTS` 不更新已有表 | 删除旧 `.db` 文件后重启 |
| **Qt 资源路径错误** | QRC 中路径相对 `.qrc` 文件所在目录 | 检查资源路径，用 `:/` 前缀访问 |
