#include "view/FormulaHint.h"
#include <QLineEdit>
#include <QCompleter>
#include <QAbstractItemView>
#include <QToolTip>

namespace formulahint {

// Vị trí bắt đầu của token tên-hàm (chuỗi chữ cái liền trước con trỏ).
static int tokenStart(const QString &text, int pos) {
    int start = pos;
    while (start > 0 && text.at(start - 1).isLetter()) --start;
    return start;
}

// Tên hàm bao quanh con trỏ (hàm có dấu '(' chưa đóng gần con trỏ nhất). Rỗng nếu không có.
static QString enclosingFunction(const QString &text, int caret) {
    int depth = 0, open = -1;
    for (int i = caret - 1; i >= 0; --i) {
        const QChar ch = text.at(i);
        if (ch == QLatin1Char(')')) ++depth;
        else if (ch == QLatin1Char('(')) { if (depth == 0) { open = i; break; } --depth; }
    }
    if (open <= 0) return QString();
    int s = open;
    while (s > 0 && (text.at(s - 1).isLetterOrNumber() || text.at(s - 1) == QLatin1Char('_'))) --s;
    return text.mid(s, open - s).toUpper();
}

void install(QLineEdit *edit, const QStringList &functionNames,
             const QHash<QString, QString> &signatures) {
    auto *completer = new QCompleter(functionNames, edit);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setWidget(edit);

    QObject::connect(edit, &QLineEdit::textEdited, edit, [edit, completer, signatures](const QString &text) {
        if (!text.startsWith(QLatin1Char('='))) { completer->popup()->hide(); QToolTip::hideText(); return; }
        const int pos = edit->cursorPosition();
        const int start = tokenStart(text, pos);
        const QString token = text.mid(start, pos - start);
        if (!token.isEmpty()) {
            completer->setCompletionPrefix(token.toUpper());
            if (completer->completionCount() > 0) { completer->complete(); return; }
        }
        completer->popup()->hide();
        // Không gợi tên hàm -> thử hiện tooltip cú pháp tham số của hàm đang ở trong ngoặc.
        const QString fn = enclosingFunction(text, pos);
        const QString sig = fn.isEmpty() ? QString() : signatures.value(fn);
        if (!sig.isEmpty())
            QToolTip::showText(edit->mapToGlobal(QPoint(0, edit->height())), sig, edit);
        else
            QToolTip::hideText();
    });

    QObject::connect(completer, QOverload<const QString &>::of(&QCompleter::activated),
                     edit, [edit](const QString &choice) {
        const int pos = edit->cursorPosition();
        QString text = edit->text();
        const int start = tokenStart(text, pos);
        text.replace(start, pos - start, choice + QLatin1Char('('));
        edit->setText(text);
        edit->setCursorPosition(start + choice.size() + 1);
    });
}

} // namespace formulahint
