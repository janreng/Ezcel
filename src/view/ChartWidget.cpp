#include "view/ChartWidget.h"
#include "ui/Theme.h"
#include <QPainter>
#include <QFont>
#include <QPolygonF>

ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(360, 240);
}

void ChartWidget::setData(const chart::Series &s, const QString &title, Type type)
{
    m_series = s;
    m_title = title;
    m_type = type;
    update();
}

void ChartWidget::paintAxes(QPainter &p, const QRect &plot, double top)
{
    const int steps = 5;
    QFont f = p.font(); f.setPointSize(8); p.setFont(f);
    for (int i = 0; i <= steps; ++i) {
        const int y = plot.bottom() - plot.height() * i / steps;
        p.setPen(QColor(theme::Gridline));
        p.drawLine(plot.left(), y, plot.right(), y);
        p.setPen(QColor("#605E5C"));
        p.drawText(QRect(0, y - 8, 48, 16), Qt::AlignRight | Qt::AlignVCenter,
                   QString::number(top * i / steps, 'g', 4));
    }
    p.setPen(QColor("#605E5C"));
    p.drawLine(plot.bottomLeft(), plot.bottomRight());
    p.drawLine(plot.topLeft(), plot.bottomLeft());
}

void ChartWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), Qt::white);

    const int W = width(), H = height();
    QFont f = p.font();
    f.setBold(true); f.setPointSize(11);
    p.setFont(f);
    p.setPen(QColor(theme::TextPrimary));
    p.drawText(QRect(0, 4, W, 22), Qt::AlignHCenter, m_title);
    f.setBold(false); f.setPointSize(8); p.setFont(f);

    if (m_series.values.isEmpty()) return;
    const int n = m_series.values.size();

    // ----- Biểu đồ tròn (pie) -----
    if (m_type == Type::Pie) {
        const QVector<double> angles = chart::pieAngles(m_series.values); // core đã kiểm thử
        if (angles.isEmpty()) return;
        const int side = qMin(W - 180, H - 60);
        const QRectF pie(20, 36, qMax(80, side), qMax(80, side));
        static const QColor palette[] = {
            QColor("#4472C4"), QColor("#ED7D31"), QColor("#A5A5A5"), QColor("#FFC000"),
            QColor("#5B9BD5"), QColor("#70AD47"), QColor("#264478"), QColor("#9E480E") };
        double start = 90.0; // bắt đầu từ trên, quay theo chiều kim đồng hồ
        const int legendX = int(pie.right()) + 24;
        int ly = int(pie.top());
        for (int i = 0; i < angles.size(); ++i) {
            const double span = angles[i];
            const QColor c = palette[i % 8];
            p.setPen(Qt::white); p.setBrush(c);
            p.drawPie(pie, int(start * 16), int(-span * 16));
            start -= span;
            p.setBrush(c); p.setPen(Qt::NoPen);
            p.drawRect(legendX, ly + 2, 12, 12);
            p.setPen(QColor(theme::TextPrimary));
            p.drawText(legendX + 18, ly + 12,
                       QStringLiteral("%1 (%2%)").arg(m_series.labels.value(i))
                           .arg(QString::number(span / 360.0 * 100, 'f', 1)));
            ly += 20;
        }
        return;
    }

    // ----- Cột / Đường -----
    const QRect plot(52, 32, W - 70, H - 70);
    if (plot.width() < 20 || plot.height() < 20) return;
    double mx = 0; for (double v : m_series.values) mx = qMax(mx, v);
    const double top = chart::niceMax(mx);
    paintAxes(p, plot, top);

    const double slot = double(plot.width()) / n;
    auto px = [&](int i) { return plot.left() + slot * i + slot / 2.0; };
    auto py = [&](double v) { return plot.bottom() - (top > 0 ? plot.height() * v / top : 0); };

    if (m_type == Type::Column) {
        const double bw = slot * 0.6;
        for (int i = 0; i < n; ++i) {
            const double bh = top > 0 ? plot.height() * m_series.values[i] / top : 0;
            const QRectF bar(plot.left() + slot * i + (slot - bw) / 2, plot.bottom() - bh, bw, bh);
            p.fillRect(bar, QColor(QStringLiteral("#4472C4")));
            p.setPen(QColor(theme::TextPrimary));
            p.drawText(QRectF(plot.left() + slot * i, bar.top() - 16, slot, 14),
                       Qt::AlignHCenter | Qt::AlignBottom, QString::number(m_series.values[i], 'g', 4));
            p.drawText(QRectF(plot.left() + slot * i, plot.bottom() + 2, slot, 16),
                       Qt::AlignHCenter, m_series.labels.value(i));
        }
    } else { // Line
        QPolygonF poly;
        for (int i = 0; i < n; ++i) poly << QPointF(px(i), py(m_series.values[i]));
        QPen pen(QColor(QStringLiteral("#4472C4"))); pen.setWidthF(2.0);
        p.setPen(pen); p.setBrush(Qt::NoBrush);
        p.drawPolyline(poly);
        p.setBrush(QColor(QStringLiteral("#4472C4"))); p.setPen(Qt::NoPen);
        for (int i = 0; i < n; ++i) p.drawEllipse(poly[i], 3, 3);
        p.setPen(QColor(theme::TextPrimary));
        for (int i = 0; i < n; ++i) {
            p.drawText(QRectF(plot.left() + slot * i, poly[i].y() - 18, slot, 14),
                       Qt::AlignHCenter | Qt::AlignBottom, QString::number(m_series.values[i], 'g', 4));
            p.drawText(QRectF(plot.left() + slot * i, plot.bottom() + 2, slot, 16),
                       Qt::AlignHCenter, m_series.labels.value(i));
        }
    }
}
