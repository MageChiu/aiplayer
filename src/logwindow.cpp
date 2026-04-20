#include "logwindow.h"

#include "logcenter.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

LogWindow::LogWindow(QWidget *parent)
    : QWidget(parent) {
    setWindowTitle(QStringLiteral("运行日志与状态"));
    resize(900, 520);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    auto *title = new QLabel(QStringLiteral("<b>当前运行状态</b>"), this);
    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_statusLabel->setStyleSheet(QStringLiteral("background:#202020;color:#f0f0f0;padding:8px;border-radius:6px;"));

    auto *logTitle = new QLabel(QStringLiteral("<b>实时日志</b>"), this);
    m_logView = new QPlainTextEdit(this);
    m_logView->setReadOnly(true);
    m_logView->setLineWrapMode(QPlainTextEdit::NoWrap);

    m_clearButton = new QPushButton(QStringLiteral("清空日志"), this);
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_clearButton);

    layout->addWidget(title);
    layout->addWidget(m_statusLabel);
    layout->addWidget(logTitle);
    layout->addWidget(m_logView, 1);
    layout->addLayout(buttonLayout);

    m_statusLabel->setText(LogCenter::instance().statusText());
    m_logView->setPlainText(LogCenter::instance().logs().join('\n'));

    connect(&LogCenter::instance(), &LogCenter::logAdded, this, [this](const QString &line) {
        m_logView->appendPlainText(line);
    });
    connect(&LogCenter::instance(), &LogCenter::statusChanged, this, [this](const QString &text) {
        m_statusLabel->setText(text);
    });
    connect(&LogCenter::instance(), &LogCenter::logsCleared, this, [this]() {
        m_logView->clear();
    });
    connect(m_clearButton, &QPushButton::clicked, this, []() {
        LogCenter::instance().clear();
    });
}
