// DatabaseManager.cpp — 数据库管理器实现

#include "DatabaseManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDebug>

DatabaseManager &DatabaseManager::instance()
{
    static DatabaseManager inst;
    return inst;
}

DatabaseManager::~DatabaseManager()
{
    close();
}

bool DatabaseManager::initialize(const QString &dbPath)
{
    if (m_initialized) return true;

    // 确定数据库文件路径
    if (dbPath.isEmpty())
    {
        // 使用系统标准数据目录：Windows → AppData/Local/WebNav
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDir);
        m_dbPath = dataDir + "/webnav.db";
    }
    else
    {
        m_dbPath = dbPath;
        QDir().mkpath(QFileInfo(m_dbPath).absolutePath());
    }

    // 打开 SQLite 数据库
    m_db = QSqlDatabase::addDatabase("QSQLITE", "webnav_connection");
    m_db.setDatabaseName(m_dbPath);

    if (!m_db.open())
    {
        qCritical() << "[DB] 数据库打开失败:" << m_db.lastError().text();
        return false;
    }

    qInfo() << "[DB] 数据库路径:" << m_dbPath;

    // 启用 WAL 模式和外键约束
    QSqlQuery query(m_db);
    query.exec("PRAGMA journal_mode=WAL");
    query.exec("PRAGMA foreign_keys=ON");

    // 执行建表和迁移
    if (!executeSchema())
    {
        qCritical() << "[DB] 建表失败";
        return false;
    }

    m_initialized = true;
    qInfo() << "[DB] 数据库初始化完成";
    return true;
}

void DatabaseManager::close()
{
    if (m_db.isOpen())
    {
        m_db.close();
    }
    m_initialized = false;
}

QSqlDatabase &DatabaseManager::database()
{
    return m_db;
}

QString DatabaseManager::databasePath() const
{
    return m_dbPath;
}

bool DatabaseManager::isInitialized() const
{
    return m_initialized;
}

bool DatabaseManager::executeSchema()
{
    // 从 Qt 资源中读取 schema.sql
    QFile schemaFile(":/database/schema.sql");
    if (!schemaFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical() << "[DB] 无法打开 schema.sql 资源文件";
        return false;
    }

    QString schemaSql = schemaFile.readAll();
    schemaFile.close();

    // 按分号分割为多条语句逐条执行
    QStringList statements = schemaSql.split(";", Qt::SkipEmptyParts);
    QSqlQuery query(m_db);

    for (const QString &stmt : statements)
    {
        QString trimmed = stmt.trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith("--"))
            continue;

        if (!query.exec(trimmed))
        {
            // 忽略 "already exists" 类错误
            if (!query.lastError().text().contains("already exists"))
            {
                qWarning() << "[DB] SQL 执行失败:" << query.lastError().text()
                           << "\n  语句:" << trimmed.left(80);
            }
        }
    }

    return true;
}

bool DatabaseManager::migrateIfNeeded()
{
    // 预留：后续版本迁移逻辑
    // 当前版本为 1，后续增加版本号时在此处添加迁移逻辑
    return true;
}
