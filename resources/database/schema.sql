-- ============================================================
-- WebNav 数据库初始化脚本
-- 版本: 1.0.0
-- ============================================================

PRAGMA journal_mode = WAL;
PRAGMA foreign_keys = ON;

-- 文件夹表
CREATE TABLE IF NOT EXISTS folders (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT NOT NULL,
    parent_id   INTEGER REFERENCES folders(id) ON DELETE CASCADE,
    sort_order  INTEGER NOT NULL DEFAULT 0,
    created_at  TEXT NOT NULL DEFAULT (datetime("now")),
    updated_at  TEXT NOT NULL DEFAULT (datetime("now"))
);

-- 标签表
CREATE TABLE IF NOT EXISTS tags (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT NOT NULL UNIQUE,
    color       TEXT NOT NULL DEFAULT "#5B9BD5",
    created_at  TEXT NOT NULL DEFAULT (datetime("now"))
);

-- 链接表
CREATE TABLE IF NOT EXISTS links (
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
    created_at      TEXT NOT NULL DEFAULT (datetime("now")),
    updated_at      TEXT NOT NULL DEFAULT (datetime("now")),
    sync_version    INTEGER NOT NULL DEFAULT 1,
    sync_updated_at TEXT
);

-- 链接-标签关联表
CREATE TABLE IF NOT EXISTS link_tags (
    link_id INTEGER NOT NULL REFERENCES links(id) ON DELETE CASCADE,
    tag_id  INTEGER NOT NULL REFERENCES tags(id) ON DELETE CASCADE,
    PRIMARY KEY (link_id, tag_id)
);

-- 链接附加字段表（账号/密码/邮箱/电话/自定义）
CREATE TABLE IF NOT EXISTS link_fields (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    link_id      INTEGER NOT NULL REFERENCES links(id) ON DELETE CASCADE,
    field_key    TEXT NOT NULL,
    field_value  TEXT NOT NULL DEFAULT "",
    field_type   INTEGER NOT NULL DEFAULT 0,
    is_password  INTEGER NOT NULL DEFAULT 0,
    sort_order   INTEGER NOT NULL DEFAULT 0,
    created_at   TEXT NOT NULL DEFAULT (datetime("now"))
);

-- 数据库版本管理表
CREATE TABLE IF NOT EXISTS schema_version (
    version     INTEGER PRIMARY KEY,
    applied_at  TEXT NOT NULL DEFAULT (datetime("now"))
);

-- 索引
CREATE INDEX IF NOT EXISTS idx_links_folder ON links(folder_id);
CREATE INDEX IF NOT EXISTS idx_links_url ON links(url);
CREATE INDEX IF NOT EXISTS idx_links_created ON links(created_at);
CREATE INDEX IF NOT EXISTS idx_link_fields_link ON link_fields(link_id);

-- 初始版本记录
INSERT OR IGNORE INTO schema_version (version) VALUES (1);
