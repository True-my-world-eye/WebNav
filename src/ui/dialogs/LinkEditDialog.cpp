// LinkEditDialog.cpp

#include "LinkEditDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QUrl>
#include <QFormLayout>
#include <QGroupBox>

LinkEditDialog::LinkEditDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
    setWindowTitle(QStringLiteral("\u65b0\u5efa\u94fe\u63a5"));
    setMinimumWidth(520);
    resize(540, 600);
}

void LinkEditDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);

    // ── URL / 标题 ──
    auto *urlLayout = new QHBoxLayout();
    urlLayout->addWidget(new QLabel(QStringLiteral("URL:"), this));
    m_urlInput = new QLineEdit(this);
    m_urlInput->setPlaceholderText(QStringLiteral("\u7c98\u8d34\u6216\u8f93\u5165\u94fe\u63a5\u5730\u5740..."));
    urlLayout->addWidget(m_urlInput);
    mainLayout->addLayout(urlLayout);

    connect(m_urlInput, &QLineEdit::textChanged, this, &LinkEditDialog::onUrlChanged);

    // 标题
    auto *titleLayout = new QHBoxLayout();
    titleLayout->addWidget(new QLabel(QStringLiteral("\u6807\u9898:"), this));
    m_titleInput = new QLineEdit(this);
    m_titleInput->setPlaceholderText(QStringLiteral("\u81ea\u52a8\u6293\u53d6\u4e2d..."));
    titleLayout->addWidget(m_titleInput);
    mainLayout->addLayout(titleLayout);

    // ── 文件夹 ──
    auto *folderLayout = new QHBoxLayout();
    folderLayout->addWidget(new QLabel(QStringLiteral("\u6587\u4ef6\u5939:"), this));
    m_folderCombo = new QComboBox(this);
    m_folderCombo->addItem(QStringLiteral("\u672a\u5206\u7c7b"), -1);
    folderLayout->addWidget(m_folderCombo, 1);
    mainLayout->addLayout(folderLayout);

    // ── 标签 ──
    m_tagSelector = new TagSelector(this);
    mainLayout->addWidget(m_tagSelector);

    // ── 备注 ──
    m_notesEdit = new QTextEdit(this);
    m_notesEdit->setPlaceholderText(QStringLiteral("\u5907\u6ce8..."));
    m_notesEdit->setMaximumHeight(80);
    mainLayout->addWidget(m_notesEdit);

    // ── 附加字段 ──
    m_fieldEditor = new FieldEditor(this);
    mainLayout->addWidget(m_fieldEditor);

    mainLayout->addStretch();

    // ── 按钮 ──
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    auto *cancelBtn = new QPushButton(QStringLiteral("\u53d6\u6d88"), this);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(cancelBtn);

    m_saveBtn = new QPushButton(QStringLiteral("\u4fdd\u5b58"), this);
    m_saveBtn->setDefault(true);
    connect(m_saveBtn, &QPushButton::clicked, this, &LinkEditDialog::onSave);
    btnLayout->addWidget(m_saveBtn);

    mainLayout->addLayout(btnLayout);
}

void LinkEditDialog::setLink(const Link &link)
{
    m_currentLink = link;
    m_editMode = true;

    m_urlInput->setText(link.url);
    m_titleInput->setText(link.title);
    m_notesEdit->setPlainText(link.notes);

    // 选择文件夹
    for (int i = 0; i < m_folderCombo->count(); i++)
    {
        if (m_folderCombo->itemData(i).toInt() == link.folderId)
        {
            m_folderCombo->setCurrentIndex(i);
            break;
        }
    }

    setWindowTitle(QStringLiteral("\u7f16\u8f91\u94fe\u63a5"));
}

Link LinkEditDialog::link() const
{
    Link link = m_currentLink;
    link.url = m_urlInput->text().trimmed();
    link.title = m_titleInput->text().trimmed();
    link.folderId = m_folderCombo->currentData().toInt();
    link.notes = m_notesEdit->toPlainText().trimmed();
    return link;
}

void LinkEditDialog::setFolders(const QVector<Folder> &folders)
{
    m_folderCombo->clear();
    m_folderCombo->addItem(QStringLiteral("\u672a\u5206\u7c7b"), -1);

    for (const auto &folder : folders)
    {
        m_folderCombo->addItem(folder.name, folder.id);
    }
}

void LinkEditDialog::setTags(const QVector<Tag> &tags)
{
    m_tagSelector->setAvailableTags(tags);
}

void LinkEditDialog::onUrlChanged(const QString &url)
{
    // 用户粘贴 URL 后自动提取域名作为标题的初步建议
    if (!m_editMode && url.startsWith("http") && m_titleInput->text().isEmpty())
    {
        QUrl parsed(url);
        QString host = parsed.host();
        // 如果 host 以 www. 开头，去掉 www.
        if (host.startsWith("www."))
            host = host.mid(4);
        m_titleInput->setPlaceholderText(host);
    }
}

void LinkEditDialog::onSave()
{
    // URL 是唯一必填字段
    if (m_urlInput->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this,
            QStringLiteral("\u63d0\u793a"),
            QStringLiteral("\u8bf7\u8f93\u5165\u94fe\u63a5\u5730\u5740\u3002"));
        m_urlInput->setFocus();
        return;
    }

    // 标题为空时自动使用 URL
    if (m_titleInput->text().trimmed().isEmpty())
    {
        m_titleInput->setText(m_urlInput->text().trimmed());
    }

    accept();
}

