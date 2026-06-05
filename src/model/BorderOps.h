#pragma once
#include <QString>

// Kẻ viền ô (Cell borders, Spec 06) — logic thuần để kiểm thử.
// Viền lưu dưới dạng chuỗi các cạnh: 'T' trên, 'L' trái, 'B' dưới, 'R' phải (vd "TLBR" = đủ 4).
// Bitmask: 1=trên, 2=trái, 4=dưới, 8=phải (khớp BorderRole của model & SpillEdges của delegate).
namespace borderops {

enum Edge { Top = 1, Left = 2, Bottom = 4, Right = 8, All = 15 };

// Chuỗi cạnh -> bitmask.
inline int fromString(const QString &s)
{
    int e = 0;
    for (QChar c : s) {
        switch (c.toUpper().unicode()) {
        case 'T': e |= Top; break;
        case 'L': e |= Left; break;
        case 'B': e |= Bottom; break;
        case 'R': e |= Right; break;
        default: break;
        }
    }
    return e;
}

// Bitmask -> chuỗi cạnh (thứ tự T,L,B,R).
inline QString toString(int e)
{
    QString s;
    if (e & Top) s += QLatin1Char('T');
    if (e & Left) s += QLatin1Char('L');
    if (e & Bottom) s += QLatin1Char('B');
    if (e & Right) s += QLatin1Char('R');
    return s;
}

// Các cạnh viền NGOÀI của ô (row,col) trong vùng chữ nhật [t..b]×[l..r].
// Ô ở mép trên/dưới/trái/phải của vùng mới nhận cạnh tương ứng.
inline int outlineEdges(int row, int col, int t, int l, int b, int r)
{
    int e = 0;
    if (row == t) e |= Top;
    if (row == b) e |= Bottom;
    if (col == l) e |= Left;
    if (col == r) e |= Right;
    return e;
}

} // namespace borderops
