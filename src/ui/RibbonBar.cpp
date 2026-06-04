#include "ui/RibbonBar.h"
#include "ui/Theme.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
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
    m_groupBtns = nullptr;
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
    // Khung nhóm: hàng nút (trên) + tiêu đề nhóm xám nhỏ (dưới).
    auto *group = new QFrame(m_tabContent);
    auto *gv = new QVBoxLayout(group);
    gv->setContentsMargins(6, 2, 6, 2);
    gv->setSpacing(2);

    auto *btnRow = new QWidget(group);
    m_groupBtns = new QHBoxLayout(btnRow);
    m_groupBtns->setContentsMargins(0, 0, 0, 0);
    m_groupBtns->setSpacing(2);
    gv->addWidget(btnRow, 1);

    auto *lbl = new QLabel(title, group);
    lbl->setAlignment(Qt::AlignHCenter);
    lbl->setStyleSheet(QStringLiteral("color: #605E5C; font-size: 8pt;"));
    gv->addWidget(lbl, 0);

    m_tabLay->addWidget(group);
    m_groupOpen = true;
    m_groupTitles[m_tabIndex].append(title);
}

QToolButton *RibbonBar::addButton(const QString &iconName, const QString &text,
                                  const std::function<void()> &onClick)
{
    if (!m_groupBtns) return nullptr;
    auto *btn = new QToolButton(m_tabContent);
    btn->setText(text);
    btn->setToolTip(text);
    btn->setAutoRaise(true);
    btn->setFocusPolicy(Qt::NoFocus);
    if (iconName.isEmpty()) {
        btn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    } else {
        btn->setIcon(QIcon(QStringLiteral(":/icons/%1.svg").arg(iconName)));
        btn->setIconSize(QSize(20, 20));
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }
    if (onClick) connect(btn, &QToolButton::clicked, this, onClick);
    m_groupBtns->addWidget(btn);
    return btn;
}

QToolButton *RibbonBar::addMenuButton(const QString &iconName, const QString &text, QMenu *menu)
{
    if (!m_groupBtns) return nullptr;
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
        btn->setIconSize(QSize(20, 20));
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }
    if (menu) btn->setMenu(menu);
    m_groupBtns->addWidget(btn);
    return btn;
}

void RibbonBar::addWidget(QWidget *w)
{
    if (m_groupBtns && w) m_groupBtns->addWidget(w);
}

void RibbonBar::addSpacer(int px)
{
    if (m_groupBtns) m_groupBtns->addSpacing(px);
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
