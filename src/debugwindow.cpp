// debugwindow.cpp
#include "debugwindow.h"
#include <QDebug>

DebugWindow::DebugWindow(QWidget *parent)
    : QWidget(parent) {
    setWindowTitle("Debug Window");
    setGeometry(100, 100, 800, 600); // Set a larger window size

    debugTextEdit = new QTextEdit(this);
    debugTextEdit->setReadOnly(true);
    debugTextEdit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere); // Enable word wrapping
    debugTextEdit->setLineWrapMode(QTextEdit::WidgetWidth); // Wrap text at widget width

    layout = new QVBoxLayout(this);
    layout->addWidget(debugTextEdit);
    setLayout(layout);

    // Set up the shortcut to toggle the debug window
    debugShortcut = new QShortcut(QKeySequence("Ctrl+Shift+D"), this);
    connect(debugShortcut, &QShortcut::activated, this, &DebugWindow::toggleVisibility);

    // Initially hide the debug window
    hide();
}

DebugWindow::~DebugWindow() {
    delete debugShortcut;
}

void DebugWindow::logMessage(const QString& message) {
    debugTextEdit->append(message);
}

void DebugWindow::toggleVisibility() {
    if (isVisible()) {
        hide();
    } else {
        show();
    }
}
