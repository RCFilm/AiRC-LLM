// debugwindow.h
#ifndef DEBUGWINDOW_H
#define DEBUGWINDOW_H

#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QShortcut>

class DebugWindow : public QWidget {
    Q_OBJECT

public:
    explicit DebugWindow(QWidget *parent = nullptr);
    ~DebugWindow();

    void logMessage(const QString& message);

private:
    QTextEdit *debugTextEdit;
    QVBoxLayout *layout;
    QShortcut *debugShortcut;

private slots:
    void toggleVisibility();
};

#endif // DEBUGWINDOW_H
