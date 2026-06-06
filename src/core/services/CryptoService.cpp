// CryptoService.cpp — 加密服务实现（Windows DPAPI）

#include "CryptoService.h"
#include <QByteArray>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#include <wincrypt.h>
#include <QPixmap>
#pragma comment(lib, "crypt32.lib")

// Windows DPAPI 加密实现
class DpapiCryptoService : public CryptoService
{
public:
    // 使用 CryptProtectData 加密字符串
    // 加密后的数据绑定当前 Windows 用户，其他用户/其他电脑无法解密
    QString encrypt(const QString &plainText) override
    {
        if (plainText.isEmpty()) return {};

        // 将 QString 转为 UTF-8 字节数组
        QByteArray plainData = plainText.toUtf8();
        DATA_BLOB inputBlob;
        inputBlob.pbData = reinterpret_cast<BYTE*>(plainData.data());
        inputBlob.cbData = static_cast<DWORD>(plainData.size());

        DATA_BLOB outputBlob = {0, nullptr};

        // 调用 Windows DPAPI 加密
        if (CryptProtectData(&inputBlob, nullptr, nullptr, nullptr, nullptr, 0, &outputBlob))
        {
            // 将加密结果转为 Base64 字符串存储
            QByteArray encrypted(reinterpret_cast<const char*>(outputBlob.pbData),
                                 static_cast<int>(outputBlob.cbData));
            QString result = QString::fromLatin1(encrypted.toBase64());
            LocalFree(outputBlob.pbData);
            return result;
        }

        qWarning() << "[Crypto] DPAPI 加密失败";
        return {};
    }

    // 使用 CryptUnprotectData 解密字符串
    QString decrypt(const QString &cipherText) override
    {
        if (cipherText.isEmpty()) return {};

        // 从 Base64 还原密文
        QByteArray encryptedData = QByteArray::fromBase64(cipherText.toLatin1());
        DATA_BLOB inputBlob;
        inputBlob.pbData = reinterpret_cast<BYTE*>(encryptedData.data());
        inputBlob.cbData = static_cast<DWORD>(encryptedData.size());

        DATA_BLOB outputBlob = {0, nullptr};

        if (CryptUnprotectData(&inputBlob, nullptr, nullptr, nullptr, nullptr, 0, &outputBlob))
        {
            QByteArray decrypted(reinterpret_cast<const char*>(outputBlob.pbData),
                                 static_cast<int>(outputBlob.cbData));
            QString result = QString::fromUtf8(decrypted);
            LocalFree(outputBlob.pbData);
            return result;
        }

        qWarning() << "[Crypto] DPAPI 解密失败";
        return {};
    }

    bool isAvailable() const override { return true; }
};

#else
// 非 Windows 平台占位实现（Phase 2 完善）
class FallbackCryptoService : public CryptoService
{
public:
    QString encrypt(const QString &plainText) override
    {
        // 临时：简单的 Base64 编码，后续替换为 Keychain/libsecret
        return QString::fromLatin1(plainText.toUtf8().toBase64());
    }

    QString decrypt(const QString &cipherText) override
    {
        return QString::fromUtf8(QByteArray::fromBase64(cipherText.toLatin1()));
    }

    bool isAvailable() const override { return true; }
};
#endif

// 工厂函数：根据平台创建对应的加密服务实例
std::unique_ptr<CryptoService> createPlatformCryptoService()
{
#ifdef Q_OS_WIN
    return std::make_unique<DpapiCryptoService>();
#else
    return std::make_unique<FallbackCryptoService>();
#endif
}
