// TagSelector.cpp — 标签选择器（带可视标签列表）

#include "TagSelector.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStringList>
#include <QScrollArea>
#include <QStyle>

TagSelector::TagSelector(QWidget *parent) : QWidget(parent)
{
    // 流式布局（单独创建，不挂接到 this）
    m_selectedLayout = new FlowLayout(nullptr, 6, 4, 4);
    m_selectedLayout->setContentsMargins(0,0,0,0);

    m_input = new QLineEdit(this);
    m_input->setPlaceholderText(QStringLiteral("输入标签名回车选择或新建..."));
    m_input->setClearButtonEnabled(true);

    m_completerModel = new QStringListModel(this);
    m_completer = new QCompleter(m_completerModel, this);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains);
    m_input->setCompleter(m_completer);

    // 组装布局：标签气泡区 + 输入框
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(4);

    auto *header = new QLabel(QStringLiteral("标签:"), this);
    layout->addWidget(header);

    // 已选标签显示在流式布局中
    auto *tagContainer = new QWidget(this);
    tagContainer->setLayout(m_selectedLayout);
    layout->addWidget(tagContainer);

    layout->addWidget(m_input);

    // 回车 → 选择或新建标签
    connect(m_input, &QLineEdit::returnPressed, this, &TagSelector::handleInputConfirmed);

    // 从补全列表点击选中 → 选择标签
    connect(m_completer, QOverload<const QString &>::of(&QCompleter::activated), this, [this](const QString &text) {
        selectTagByName(text);
    });
}

bool TagSelector::selectTagByName(const QString &name)
{
    for (const auto &tag : m_availableTags) {
        if (tag.name == name) {
            if (!m_selectedIds.contains(tag.id)) {
                m_selectedIds.insert(tag.id);
                rebuildTagChips();
                emit tagsChanged();
            }
            m_input->clear();
            return true;
        }
    }
    return false;
}

void TagSelector::handleInputConfirmed()
{
    QString text = m_input->text().trimmed();
    if (text.isEmpty()) return;

    // 先尝试匹配已有标签
    if (selectTagByName(text))
        return;

    // 未匹配：创建新标签
    emit createNewTag(text);
    m_input->clear();
}

void TagSelector::setAvailableTags(const QVector<Tag> &tags)
{
    m_availableTags = tags;
    QStringList names;
    for (const auto &t : tags) names << t.name;
    m_completerModel->setStringList(names);
}

void TagSelector::setSelectedTagIds(const QVector<int> &ids)
{
    m_selectedIds.clear();
    m_selectedIds.reserve(ids.size());
    for (int id : ids) m_selectedIds.insert(id);
    rebuildTagChips();
}

void TagSelector::addSelectedTagId(int id)
{
    m_selectedIds.insert(id);
    rebuildTagChips();
}

void TagSelector::rebuildTagChips()
{
    // 清除旧的 chips
    QLayoutItem *child;
    while ((child = m_selectedLayout->takeAt(0)) != nullptr) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }

    // 为每个已选标签创建一个 chip
    for (int id : m_selectedIds) {
        QString name;
        for (const auto &tag : m_availableTags) {
            if (tag.id == id) {
                name = tag.name;
                break;
            }
        }
        if (name.isEmpty()) {
            // 可能是新建标签尚未刷新 availableTags
            name = QStringLiteral("#%1").arg(id);
        }

        auto *chip = new QPushButton(name, this);
        chip->setFixedHeight(24);
        chip->setStyleSheet(
            "QPushButton {"
            "  background: #e0e7ff; border: 1px solid #a5b4fc;"
            "  border-radius: 10px; padding: 0 8px;"
            "  font-size: 12px; color: #4338ca;"
            "}"
            "QPushButton:hover { background: #c7d2fe; }"
        );
        chip->setCursor(Qt::PointingHandCursor);
        connect(chip, &QPushButton::clicked, this, [this, id]() {
            m_selectedIds.remove(id);
            rebuildTagChips();
            emit tagsChanged();
        });
        m_selectedLayout->addWidget(chip);
    }
}

// ── FlowLayout 实现 ──────────────────────────────────

FlowLayout::FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::~FlowLayout()
{
    QLayoutItem *item;
    while ((item = takeAt(0)))
        delete item;
}

void FlowLayout::addItem(QLayoutItem *item)
{
    m_itemList.append(item);
}

int FlowLayout::count() const
{
    return m_itemList.size();
}

QLayoutItem *FlowLayout::itemAt(int index) const
{
    return m_itemList.value(index);
}

QLayoutItem *FlowLayout::takeAt(int index)
{
    if (index >= 0 && index < m_itemList.size())
        return m_itemList.takeAt(index);
    return nullptr;
}

Qt::Orientations FlowLayout::expandingDirections() const
{
    return {};
}

bool FlowLayout::hasHeightForWidth() const
{
    return true;
}

int FlowLayout::heightForWidth(int width) const
{
    return doLayout(QRect(0, 0, width, 0), true);
}

QSize FlowLayout::sizeHint() const
{
    return minimumSize();
}

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for (const QLayoutItem *item : m_itemList)
        size = size.expandedTo(item->minimumSize());
    const auto margins = contentsMargins();
    size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    return size;
}

void FlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

int FlowLayout::doLayout(const QRect &rect, bool testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    for (QLayoutItem *item : m_itemList) {
        const QWidget *wid = item->widget();
        int spaceX = m_hSpace;
        int spaceY = m_vSpace;
        if (wid) {
            spaceX = qMax(spaceX, wid->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal));
            spaceY = qMax(spaceY, wid->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical));
        }
        int nextX = x + item->sizeHint().width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
            lineHeight = 0;
        }
        if (!testOnly)
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));
        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    return y + lineHeight - rect.y() + bottom;
}
