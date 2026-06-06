// CryptoService.h — 加密服务
// 提供密码类字段的加密/解密功能，各平台使用不同的后端实现
// Windows: DPAPI (CryptProtectData)
// macOS:   Keychain Services
// Linux:   libsecret / AES-256-GCM

#pragma once
#include <QString>

// 加密服务抽象基类
class CryptoService
{
public:
    virtual ~CryptoService() = default;

    // 加密明文，返回 Base64 编码的密文
    virtual QString encrypt(const QString &plainText) = 0;

    // 解密 Base64 密文，返回明文
    virtual QString decrypt(const QString &cipherText) = 0;

    // 判断当前平台加密是否可用
    virtual bool isAvailable() const = 0;
};

// Windows DPAPI 实现工厂
// 使用 Win32 CryptProtectData 加密，绑定当前 Windows 用户凭据
std::unique_ptr<CryptoService> createPlatformCryptoService();
