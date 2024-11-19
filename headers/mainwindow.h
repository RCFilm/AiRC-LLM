#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QDialog>
#include <QFormLayout>
#include <QComboBox>
#include "headers/Ollama.h" // Include the ollama-hpp header

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addWorkspace();
    void selectWorkspace(QListWidgetItem *item);
    void sendMessage();
    void clearChat();
    void openSettings(QListWidgetItem *item);
    void loadModelList();

    // New slots for network replies
    void onModelLoaded(QNetworkReply* reply);
    void onModelListFetched(QNetworkReply* reply);
    void onInferenceFinished(QNetworkReply* reply);
    void onInferenceDataReady(QNetworkReply* reply);

private:
    Ui::MainWindow *ui;
    QWidget *centralWidget;
    QHBoxLayout *mainLayout;
    QVBoxLayout *leftLayout;
    QVBoxLayout *rightLayout;
    QPushButton *addWorkspaceButton;
    QListWidget *workspacesList;
    QTextEdit *chatTextEdit;
    QLineEdit *inputLineEdit;
    QPushButton *sendButton;
    QPushButton *clearButton;
    std::map<QString, QString> workspaceModels;
    QNetworkAccessManager *networkManager;
};

#endif // MAINWINDOW_H
