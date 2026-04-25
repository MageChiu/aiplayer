#pragma once

#include <QWidget>

class QLabel;
class QPushButton;
class QPlainTextEdit;

class LogWindow : public QWidget {
    Q_OBJECT

public:
    explicit LogWindow(QWidget *parent = nullptr);

private:
    QLabel *m_statusLabel = nullptr;
    QPlainTextEdit *m_logView = nullptr;
    QPushButton *m_clearButton = nullptr;
};
