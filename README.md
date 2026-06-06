# WebNav — 桌面网页链接管理器

> 基于 C++ / Qt 6 的桌面网页链接管理器
> 双视图可切换 · 链接账密存储 · 可扩展云端同步

## 项目状态

🚧 Phase 1 开发中 — 核心体验模块

### 已完成的模块

- [x] 项目骨架（CMake 构建、目录结构、Git）
- [ ] 数据模型
- [ ] 数据库层
- [ ] 核心服务
- [ ] UI 主窗口
- [ ] UI 侧边栏
- [ ] UI 双视图
- [ ] 编辑对话框
- [ ] 导入导出
- [ ] 应用层/主题/快捷键

## 环境要求

- Qt 6.5+
- CMake 3.20+
- 编译器：MSVC 2022 / MinGW 11+ / GCC 11+

## 编译运行

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt6
cmake --build .
./WebNav
```

## 功能特色

详见 [design.md](design.md)。
