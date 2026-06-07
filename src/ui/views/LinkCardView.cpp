// LinkCardView.cpp — 卡片视图（带自定义绘制代理）

#include "LinkCardView.h"
#include <QPainter>
#include <QApplication>
#include <QStyledItemDelegate>

// ── LinkCardView ──────────────────────────────────

LinkCardView::LinkCardView(QWidget *parent)
    : QListView(parent)
{
    setViewMode(QListView::IconMode);
    setResizeMode(QListView::Adjust);
    setGridSize(QSize(200, 160));
    setSpacing(10);
    setWordWrap(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setItemDelegate(new LinkCardDelegate(this));

    connect(this, &QListView::doubleClicked, this, [this](const QModelIndex &index) {
        if (index.isValid())
        {
            int linkId = index.data(Qt::UserRole).toInt();
            emit linkDoubleClicked(linkId);
        }
    });
}

void LinkCardView::setLinkData(QStandardItemModel *model)
{
    m_model = model;
    setModel(m_model);
}

// ── LinkCardDelegate ────────────────────────────

LinkCardDelegate::LinkCardDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QSize LinkCardDelegate::sizeHint(const QStyleOptionViewItem & /*option*/,
                                  const QModelIndex & /*index*/) const
{
    return QSize(180, 150);
}

void LinkCardDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    painter->save();

    // 获取数据
    QString title = index.sibling(index.row(), 0).data(Qt::DisplayRole).toString();
    QString url   = index.sibling(index.row(), 1).data(Qt::DisplayRole).toString();
    QString tags  = index.sibling(index.row(), 3).data(Qt::DisplayRole).toString();

    // 域名提取
    QString domain;
    if (url.startsWith("http")) {
        int slash = url.indexOf('/', 8);
        domain = (slash > 0) ? url.mid(0, slash) : url;
        domain.remove("https://").remove("http://").remove("www.");
        if (domain.length() > 30) domain = domain.left(28) + "...";
    } else {
        domain = url.left(30);
    }

    QRect r = option.rect;
    bool isDark = false;
    if (QApplication::palette().window().color().lightness() < 128)
        isDark = true;

    // ── 卡片背景 ──
    QColor bg = isDark ? QColor("#2c2c2e") : QColor("#ffffff");
    QColor border = isDark ? QColor("#38383a") : QColor("#e8e8ed");

    if (option.state & QStyle::State_Selected) {
        bg = isDark ? QColor("#3a3a3c") : QColor("#e8e8ed");
        border = isDark ? QColor("#636366") : QColor("#c7c7cc");
    }

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(bg);
    painter->setPen(QPen(border, 1));
    painter->drawRoundedRect(r.adjusted(2, 2, -2, -2), 10, 10);

    // ── 上方的占位图标（简化 favicon 占位） ──
    QRect iconRect(r.left() + r.width()/2 - 18, r.top() + 16, 36, 36);
    painter->setBrush(isDark ? QColor("#48484a") : QColor("#e8e8ed"));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(iconRect, 8, 8);
    // 简化图标：画一个地球符号
    painter->setPen(isDark ? QColor("#98989d") : QColor("#8e8e93"));
    QFont iconFont = painter->font();
    iconFont.setPixelSize(18);
    painter->setFont(iconFont);
    painter->drawText(iconRect, Qt::AlignCenter, QStringLiteral("\u{1F310}"));

    // ── 标题 ──
    QRect titleRect(r.left() + 8, iconRect.bottom() + 6, r.width() - 16, 36);
    painter->setPen(isDark ? QColor("#f5f5f7") : QColor("#1d1d1f"));
    QFont titleFont = painter->font();
    titleFont.setPixelSize(12);
    titleFont.setBold(true);
    painter->setFont(titleFont);
    QString displayTitle = title.isEmpty() ? domain : title;
    if (painter->fontMetrics().horizontalAdvance(displayTitle) > titleRect.width()) {
        displayTitle = painter->fontMetrics().elidedText(displayTitle, Qt::ElideRight, titleRect.width());
    }
    painter->drawText(titleRect, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, displayTitle);

    // ── 域名 ──
    QRect domainRect(r.left() + 8, titleRect.bottom() + 2, r.width() - 16, 18);
    painter->setPen(isDark ? QColor("#98989d") : QColor("#8e8e93"));
    QFont domainFont = painter->font();
    domainFont.setPixelSize(10);
    domainFont.setBold(false);
    painter->setFont(domainFont);
    if (painter->fontMetrics().horizontalAdvance(domain) > domainRect.width()) {
        domain = painter->fontMetrics().elidedText(domain, Qt::ElideRight, domainRect.width());
    }
    painter->drawText(domainRect, Qt::AlignCenter, domain);

    // ── 标签 ──
    if (!tags.isEmpty()) {
        QRect tagsRect(r.left() + 8, domainRect.bottom() + 4, r.width() - 16, 20);
        painter->setPen(QColor(isDark ? "#0a84ff" : "#0071e3"));
        QFont tagFont = painter->font();
        tagFont.setPixelSize(9);
        painter->setFont(tagFont);
        QString displayTags = tags;
        if (painter->fontMetrics().horizontalAdvance(displayTags) > tagsRect.width()) {
            displayTags = painter->fontMetrics().elidedText(displayTags, Qt::ElideRight, tagsRect.width());
        }
        painter->drawText(tagsRect, Qt::AlignCenter, displayTags);
    }

    painter->restore();
}
