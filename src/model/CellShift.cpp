#include "model/CellShift.h"

namespace cellshift {

QVector<Move> plan(int rows, int cols, int top, int left, int bottom, int right, Dir dir)
{
    QVector<Move> out;
    if (rows <= 0 || cols <= 0) return out;
    if (top < 0 || left < 0 || bottom >= rows || right >= cols) return out;
    if (top > bottom || left > right) return out;

    const int H = bottom - top + 1; // chiều cao vùng chọn
    const int W = right - left + 1; // chiều rộng vùng chọn
    // Quy ước: srcR < 0 nghĩa là ô đích để TRỐNG.

    switch (dir) {
    case Dir::Right:
        // Mỗi hàng [top,bottom]: cột >= left dời sang phải W; chừa khoảng trống ở [left,right].
        for (int r = top; r <= bottom; ++r)
            for (int c = left; c < cols; ++c) {
                int src = c - W;
                if (src >= left) out.push_back({r, c, r, src});
                else             out.push_back({r, c, -1, -1});
            }
        break;
    case Dir::Left:
        // Mỗi hàng [top,bottom]: xóa [left,right], dồn cột bên phải sang trái W.
        for (int r = top; r <= bottom; ++r)
            for (int c = left; c < cols; ++c) {
                int src = c + W;
                if (src < cols) out.push_back({r, c, r, src});
                else            out.push_back({r, c, -1, -1});
            }
        break;
    case Dir::Down:
        // Mỗi cột [left,right]: hàng >= top dời xuống H; chừa khoảng trống ở [top,bottom].
        for (int c = left; c <= right; ++c)
            for (int r = top; r < rows; ++r) {
                int src = r - H;
                if (src >= top) out.push_back({r, c, src, c});
                else            out.push_back({r, c, -1, -1});
            }
        break;
    case Dir::Up:
        // Mỗi cột [left,right]: xóa [top,bottom], dồn hàng bên dưới lên trên H.
        for (int c = left; c <= right; ++c)
            for (int r = top; r < rows; ++r) {
                int src = r + H;
                if (src < rows) out.push_back({r, c, src, c});
                else            out.push_back({r, c, -1, -1});
            }
        break;
    }
    return out;
}

} // namespace cellshift
