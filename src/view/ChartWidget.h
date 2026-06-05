#pragma once
#include <QWidget>
#include <QString>
#include "model/Chart.h"

// Vẽ biểu đồ cột bằng QPainter (Spec 19) — không phụ thuộc Qt Charts.
class ChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChartWidget(QWidget *parent = nullptr);
    void setData(const chart::Series &s, const QString &title);

protected:
    void paintEvent(QPaintEvent *ev) override;

private:
    chart::Series m_series;
    QString m_title;
};
