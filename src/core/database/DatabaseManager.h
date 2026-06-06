// DatabaseManager.h — 数据库管理器
// 负责 SQLite 数据库的初始化、版本迁移、连接生命周期管理

#pragma once
#include <QString>
#include <QSqlDatabase>

// 数据库管理器（单例模式）
// 应用启动时调用 initialize() 完成建库和迁移
class DatabaseManager
{
public:
    // 获取单例实例
    static DatabaseManager &instance();

    // 初始化数据库：打开连接、执行建表迁移
    // @param dbPath  数据库文件路径，为空则使用默认路径
    bool initialize(const QString &dbPath = QString());

    // 关闭数据库连接
    void close();

    // 获取数据库连接引用
    QSqlDatabase &database();

    // 获取数据库文件路径
    QString databasePath() const;

    // 判断数据库是否已初始化
    bool isInitialized() const;

private:
    DatabaseManager() = default;
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    // 执行 schema.sql 中的建表语句
    bool executeSchema();

    // 检查并执行版本迁移
    bool migrateIfNeeded();

    QSqlDatabase m_db;                  // SQLite 数据库连接
    QString m_dbPath;                   // 数据库文件路径
    bool m_initialized = false;         // 初始化状态标记
};
