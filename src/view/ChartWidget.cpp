#include "view/ChartWidget.h"
#include "ui/Theme.h"
#include <QPainter>
#include <QFont>

ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(360, 240);
}

void ChartWidget::setData(const chart::Series &s, const QString &title)
{
    m_series = s;
    m_title = title;
    update();
}

void ChartWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), Qt::white);

    const int W = width(), H = height();
    // Tiêu đề.
    QFont f = p.font();
    f.setBold(true); f.setPointSize(11);
    p.setFont(f);
    p.setPen(QColor(theme::TextPrimary));
    p.drawText(QRect(0, 4, W, 22), Qt::AlignHCenter, m_title);
    f.setBold(false); f.setPointSize(8);
    p.setFont(f);

    if (m_series.values.isEmpty()) return;

    const QRect plot(52, 32, W - 70, H - 70); // chừa trục trái + nhãn dưới + tiêu đề
    if (plot.width() < 20 || plot.height() < 20) return;

    double mx = 0;
    for (double v : m_series.values) mx = qMax(mx, v);
    const double top = chart::niceMax(mx);

    // Lưới ngang + nhãn trục Y.
    const int steps = 5;
    for (int i = 0; i <= steps; ++i) {
        const int y = plot.bottom() - plot.height() * i / steps;
        p.setPen(QColor(theme::Gridline));
        p.drawLine(plot.left(), y, plot.right(), y);
        p.setPen(QColor("#605E5C"));
        p.drawText(QRect(0, y - 8, 48, 16), Qt::AlignRight | Qt::AlignVCenter,
                   QString::number(top * i / steps, 'g', 4));
    }

    // Cột.
    const int n = m_series.values.size();
    const double slot = double(plot.width()) / n;
    const double bw = slot * 0.6;
    for (int i = 0; i < n; ++i) {
        const double frac = top > 0 ? m_series.values[i] / top : 0;
        const int bh = int(plot.height() * frac);
        const QRectF bar(plot.left() + slot * i + (slot - bw) / 2, plot.bottom() - bh, bw, bh);
        p.fillRect(bar, QColor(QStringLiteral("#4472C4")));
        p.setPen(QColor(theme::TextPrimary));
        p.drawText(QRectF(plot.left() + slot * i, bar.top() - 16, slot, 14),
                   Qt::AlignHCenter | Qt::AlignBottom, QString::number(m_series.values[i], 'g', 4));
        p.drawText(QRectF(plot.left() + slot * i, plot.bottom() + 2, slot, 16),
                   Qt::AlignHCenter, m_series.labels.value(i));
    }

    // Trục.
    p.setPen(QColor("#605E5C"));
    p.drawLine(plot.bottomLeft(), plot.bottomRight());
    p.drawLine(plot.topLeft(), plot.bottomLeft());
}
