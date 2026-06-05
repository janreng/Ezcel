#pragma once
#include <QWidget>
#include <QString>
#include "model/Chart.h"

// Vẽ biểu đồ cột bằng QPainter (Spec 19) — không phụ thuộc Qt Charts.
class ChartWidget : public QWidget
{
    Q_OBJECT
public:
    enum class Type { Column, Line, Pie };
    explicit ChartWidget(QWidget *parent = nullptr);
    void setData(const chart::Series &s, const QString &title, Type type = Type::Column);

protected:
    void paintEvent(QPaintEvent *ev) override;

private:
    void paintAxes(QPainter &p, const QRect &plot, double top); // trục + lưới + nhãn Y
    chart::Series m_series;
    QString m_title;
    Type m_type = Type::Column;
};
