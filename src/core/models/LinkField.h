// LinkField.h — 链接附加字段数据模型
// 用于存储链接关联的登录凭据等附加信息，支持任意自定义字段扩展
// 预设字段：账号/密码/邮箱/电话，可自由添加更多字段

#pragma once
#include <QString>
#include <QDateTime>

// 链接附加字段数据结构
struct LinkField
{
    int id = -1;                        // 数据库主键
    int linkId = -1;                    // 所属链接 ID
    QString fieldKey;                   // 字段名，如 "account" "password" "email" "phone" 或自定义名称
    QString fieldValue;                 // 字段值（密码类字段加密存储，明文类直接存储原文）
    int fieldType = 0;                  // 字段类型：0=明文文本  1=加密存储  2=多行文本
    bool isPassword = false;            // 是否为密码类型（界面用 ● 遮盖显示）
    int sortOrder = 0;                  // 排序序号，控制界面显示顺序
    QDateTime createdAt;                // 创建时间

    bool isValid() const { return id > 0 && !fieldKey.isEmpty(); }
};
