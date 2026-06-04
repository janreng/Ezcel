#include "ui/RibbonBar.h"
#include "ui/Theme.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QToolButton>
#include <QLabel>
#include <QFrame>
#include <QMenu>
#include <QIcon>
#include <QSize>

RibbonBar::RibbonBar(QWidget *parent) : QTabWidget(parent)
{
    setDocumentMode(true);
    setFocusPolicy(Qt::NoFocus);
    // Dải lệnh chỉ cao bằng nội dung (không giãn nuốt vùng lưới bên dưới).
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    // Dải lệnh nền xám nhạt giống Office; tab phẳng, nhóm có tiêu đề xám nhỏ.
    setStyleSheet(QStringLiteral(
        "RibbonBar::pane { background: %1; border: none; border-bottom: 1px solid %2; }"
        "RibbonBar > QTabBar::tab { background: transparent; padding: 5px 14px;"
        "  margin-right: 2px; color: %3; border: none; }"
        "RibbonBar > QTabBar::tab:selected { background: %1; color: %4;"
        "  border-bottom: 2px solid %4; font-weight: 600; }"
        "RibbonBar > QTabBar::tab:hover { color: %4; }"
        "QToolButton { border: none; border-radius: 3px; padding: 3px; }"
        "QToolButton:hover { background: %5; }"
        "QToolButton:pressed { background: %2; }")
        .arg(theme::RibbonBg, theme::Divider, theme::TextPrimary,
             theme::ExcelGreen, theme::HoverBg));
}

void RibbonBar::closeCurrentTab()
{
    if (m_tabLay) m_tabLay->addStretch(); // dồn các nhóm về trái
}

void RibbonBar::beginTab(const QString &title)
{
    closeCurrentTab();
    m_tabContent = new QWidget(this);
    m_tabContent->setStyleSheet(QStringLiteral("background: %1;").arg(theme::RibbonBg));
    m_tabLay = new QHBoxLayout(m_tabContent);
    m_tabLay->setContentsMargins(6, 4, 6, 2);
    m_tabLay->setSpacing(0);
    m_tabIndex = addTab(m_tabContent, title);
    m_groupOpen = false;
    m_groupGrid = nullptr;
    m_col = 0;
    m_smallRow = 0;
    m_groupTitles.insert(m_tabIndex, QStringList());
}

void RibbonBar::beginGroup(const QString &title)
{
    if (!m_tabLay) return;
    if (m_groupOpen) { // vạch dọc ngăn nhóm trước
        auto *sep = new QFrame(m_tabContent);
        sep->setFrameShape(QFrame::VLine);
        sep->setStyleSheet(QStringLiteral("color: %1;").arg(theme::Divider));
        m_tabLay->addWidget(sep);
    }
    // Khung nhóm: lưới nút 3 hàng (trên) + tiêu đề nhóm xám nhỏ (dưới).
    auto *group = new QFrame(m_tabContent);
    auto *gv = new QVBoxLayout(group);
    gv->setContentsMargins(6, 2, 6, 2);
    gv->setSpacing(2);

    auto *btnArea = new QWidget(group);
    m_groupGrid = new QGridLayout(btnArea);
    m_groupGrid->setContentsMargins(0, 0, 0, 0);
    m_groupGrid->setHorizontalSpacing(2);
    m_groupGrid->setVerticalSpacing(1);
    gv->addWidget(btnArea, 1);

    auto *lbl = new QLabel(title, group);
    lbl->setAlignment(Qt::AlignHCenter);
    lbl->setStyleSheet(QStringLiteral("color: #605E5C; font-size: 8pt;"));
    gv->addWidget(lbl, 0);

    m_tabLay->addWidget(group, 0, Qt::AlignTop);
    m_groupOpen = true;
    m_col = 0;
    m_smallRow = 0;
    m_groupTitles[m_tabIndex].append(title);
}

void RibbonBar::closeSmallColumn()
{
    if (m_smallRow != 0) { m_smallRow = 0; ++m_col; }
}

QToolButton *RibbonBar::addButton(const QString &iconName, const QString &text,
                                  const std::function<void()> &onClick)
{
    if (!m_groupGrid) return nullptr;
    closeSmallColumn();
    auto *btn = new QToolButton(m_tabContent);
    btn->setText(text);
    btn->setToolTip(text);
    btn->setAutoRaise(true);
    btn->setFocusPolicy(Qt::NoFocus);
    if (iconName.isEmpty()) {
        btn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    } else {
        btn->setIcon(QIcon(QStringLiteral(":/icons/%1.svg").arg(iconName)));
        btn->setIconSize(QSize(28, 28));
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }
    if (onClick) connect(btn, &QToolButton::clicked, this, onClick);
    m_groupGrid->addWidget(btn, 0, m_col, 3, 1); // nút lớn chiếm trọn 3 hàng
    ++m_col;
    return btn;
}

QToolButton *RibbonBar::addSmallButton(const QString &iconName, const QString &text,
                                       const std::function<void()> &onClick)
{
    if (!m_groupGrid) return nullptr;
    auto *btn = new QToolButton(m_tabContent);
    btn->setText(text);
    btn->setToolTip(text);
    btn->setAutoRaise(true);
    btn->setFocusPolicy(Qt::NoFocus);
    if (iconName.isEmpty()) {
        btn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    } else {
        btn->setIcon(QIcon(QStringLiteral(":/icons/%1.svg").arg(iconName)));
        btn->setIconSize(QSize(16, 16));
        btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }
    if (onClick) connect(btn, &QToolButton::clicked, this, onClick);
    m_groupGrid->addWidget(btn, m_smallRow, m_col, Qt::AlignLeft | Qt::AlignVCenter);
    if (++m_smallRow >= 3) { m_smallRow = 0; ++m_col; }
    return btn;
}

QToolButton *RibbonBar::addMenuButton(const QString &iconName, const QString &text, QMenu *menu)
{
    if (!m_groupGrid) return nullptr;
    closeSmallColumn();
    auto *btn = new QToolButton(m_tabContent);
    btn->setText(text);
    btn->setToolTip(text);
    btn->setAutoRaise(true);
    btn->setFocusPolicy(Qt::NoFocus);
    btn->setPopupMode(QToolButton::InstantPopup);
    if (iconName.isEmpty()) {
        btn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    } else {
        btn->setIcon(QIcon(QStringLiteral(":/icons/%1.svg").arg(iconName)));
        btn->setIconSize(QSize(28, 28));
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }
    if (menu) btn->setMenu(menu);
    m_groupGrid->addWidget(btn, 0, m_col, 3, 1); // nút thả lớn chiếm trọn 3 hàng
    ++m_col;
    return btn;
}

void RibbonBar::addWidget(QWidget *w)
{
    if (!m_groupGrid || !w) return;
    m_groupGrid->addWidget(w, m_smallRow, m_col);
    if (++m_smallRow >= 3) { m_smallRow = 0; ++m_col; }
}

void RibbonBar::addSpacer(int)
{
    closeSmallColumn(); // sang cột mới (kết thúc cụm nút nhỏ đang dở)
}

void RibbonBar::finish()
{
    closeCurrentTab();
}

QStringList RibbonBar::tabTitles() const
{
    QStringList out;
    for (int i = 0; i < count(); ++i) out << tabText(i);
    return out;
}

QStringList RibbonBar::groupTitles(int tabIndex) const
{
    return m_groupTitles.value(tabIndex);
}
