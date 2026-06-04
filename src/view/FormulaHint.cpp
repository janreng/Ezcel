#include "view/FormulaHint.h"
#include <QLineEdit>
#include <QCompleter>
#include <QAbstractItemView>

namespace formulahint {

// Vị trí bắt đầu của token tên-hàm (chuỗi chữ cái liền trước con trỏ).
static int tokenStart(const QString &text, int pos) {
    int start = pos;
    while (start > 0 && text.at(start - 1).isLetter()) --start;
    return start;
}

void install(QLineEdit *edit, const QStringList &functionNames) {
    auto *completer = new QCompleter(functionNames, edit);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setWidget(edit);

    QObject::connect(edit, &QLineEdit::textEdited, edit, [edit, completer](const QString &text) {
        if (!text.startsWith(QLatin1Char('='))) { completer->popup()->hide(); return; }
        const int pos = edit->cursorPosition();
        const int start = tokenStart(text, pos);
        const QString token = text.mid(start, pos - start);
        if (token.isEmpty()) { completer->popup()->hide(); return; }
        completer->setCompletionPrefix(token.toUpper());
        if (completer->completionCount() == 0) { completer->popup()->hide(); return; }
        completer->complete(); // xổ popup ngay dưới ô nhập
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
