// main.cpp
#include <QApplication>
#include "mainwindow.h"
#include "debugwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.setWindowTitle("AiRC-LLM"); // Set the window title
    mainWindow.show();

    return app.exec();
}
