#include "LinkEditDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QUrl>
#include <QLabel>

LinkEditDialog::LinkEditDialog(QWidget *parent) : QDialog(parent) {
    setupUi();
    setWindowTitle(QStringLiteral("\u65b0\u5efa\u94fe\u63a5"));
    setMinimumWidth(520);
    resize(540, 600);
}

void LinkEditDialog::setupUi()
{
    auto *main = new QVBoxLayout(this); main->setSpacing(8);

    // URL
    auto *urlLay = new QHBoxLayout();
    urlLay->addWidget(new QLabel("URL:", this));
    m_urlInput = new QLineEdit(this);
    m_urlInput->setPlaceholderText(QStringLiteral("\u7c98\u8d34\u6216\u8f93\u5165\u94fe\u63a5\u5730\u5740..."));
    urlLay->addWidget(m_urlInput);
    main->addLayout(urlLay);
    connect(m_urlInput, &QLineEdit::textChanged, this, [this](const QString &url) {
        if (!m_editMode && url.startsWith("http") && m_titleInput->text().isEmpty()) {
            QString host = QUrl(url).host();
            if (host.startsWith("www.")) host = host.mid(4);
            m_titleInput->setPlaceholderText(host);
        }
    });

    // 标题
    auto *titleLay = new QHBoxLayout();
    titleLay->addWidget(new QLabel(QStringLiteral("\u6807\u9898:"), this));
    m_titleInput = new QLineEdit(this);
    m_titleInput->setPlaceholderText(QStringLiteral("\u81ea\u52a8\u6293\u53d6\u4e2d..."));
    titleLay->addWidget(m_titleInput);
    main->addLayout(titleLay);

    // 文件夹
    auto *folderLay = new QHBoxLayout();
    folderLay->addWidget(new QLabel(QStringLiteral("\u6587\u4ef6\u5939:"), this));
    m_folderCombo = new QComboBox(this);
    m_folderCombo->addItem(QStringLiteral("\u672a\u5206\u7c7b"), -1);
    folderLay->addWidget(m_folderCombo, 1);
    main->addLayout(folderLay);

    // 标签
    m_tagSelector = new TagSelector(this);
    main->addWidget(m_tagSelector);

    // 备注
    m_notesEdit = new QTextEdit(this);
    m_notesEdit->setPlaceholderText(QStringLiteral("\u5907\u6ce8..."));
    m_notesEdit->setMaximumHeight(80);
    main->addWidget(m_notesEdit);

    // 时间信息
    auto *timeLay = new QHBoxLayout();
    m_timeLabel = new QLabel(this);
    m_timeLabel->setObjectName("timeInfo");
    m_timeLabel->setStyleSheet("color: #888; font-size: 11px;");
    timeLay->addWidget(m_timeLabel);
    timeLay->addStretch();
    main->addLayout(timeLay);

    // 附加字段
    m_fieldEditor = new FieldEditor(this);
    main->addWidget(m_fieldEditor);
    main->addStretch();

    // 按钮
    auto *btnLay = new QHBoxLayout();
    btnLay->addStretch();
    auto *cancelBtn = new QPushButton(QStringLiteral("\u53d6\u6d88"), this);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLay->addWidget(cancelBtn);
    m_saveBtn = new QPushButton(QStringLiteral("\u4fdd\u5b58"), this);
    m_saveBtn->setDefault(true);
    connect(m_saveBtn, &QPushButton::clicked, this, &LinkEditDialog::onSave);
    btnLay->addWidget(m_saveBtn);
    main->addLayout(btnLay);
}

void LinkEditDialog::setLink(const Link &link)
{
    m_currentLink = link; m_editMode = true;
    m_urlInput->setText(link.url);
    m_titleInput->setText(link.title);
    m_notesEdit->setPlainText(link.notes);
    for (int i = 0; i < m_folderCombo->count(); i++)
        if (m_folderCombo->itemData(i).toInt() == link.folderId)
            { m_folderCombo->setCurrentIndex(i); break; }
    setWindowTitle(QStringLiteral("\u7f16\u8f91\u94fe\u63a5"));
}

Link LinkEditDialog::link() const
{
    Link l = m_currentLink;
    l.url = m_urlInput->text().trimmed();
    l.title = m_titleInput->text().trimmed();
    l.folderId = m_folderCombo->currentData().toInt();
    l.notes = m_notesEdit->toPlainText().trimmed();
    return l;
}

void LinkEditDialog::setFolders(const QVector<Folder> &folders)
{
    m_folderCombo->clear();
    m_folderCombo->addItem(QStringLiteral("\u672a\u5206\u7c7b"), -1);
    for (const auto &f : folders)
        m_folderCombo->addItem(f.name, f.id);
}

void LinkEditDialog::setTags(const QVector<Tag> &tags)
{
    m_tagSelector->setAvailableTags(tags);
}

void LinkEditDialog::setLinkTime(const QDateTime &created, const QDateTime &updated)
{
    QString text;
    if (created.isValid())
        text += QStringLiteral("创建: %1").arg(created.toString("yyyy-MM-dd HH:mm"));
    if (updated.isValid() && updated != created) {
        if (!text.isEmpty()) text += QStringLiteral("  |  ");
        text += QStringLiteral("更新: %1").arg(updated.toString("yyyy-MM-dd HH:mm"));
    }
    m_timeLabel->setText(text);
}

void LinkEditDialog::onSave()
{
    if (m_urlInput->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("\u63d0\u793a"), QStringLiteral("\u8bf7\u8f93\u5165\u94fe\u63a5\u5730\u5740\u3002"));
        m_urlInput->setFocus();
        return;
    }
    if (m_titleInput->text().trimmed().isEmpty())
        m_titleInput->setText(m_urlInput->text().trimmed());
    accept();
}
