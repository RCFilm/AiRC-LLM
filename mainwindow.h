#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <string>

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
    void sendMessage();
    void onModelListFetched(QNetworkReply *reply);
    void onModelLoaded(QNetworkReply *reply);
    void onInferenceDataReady(QNetworkReply *reply);
    void onInferenceFinished(QNetworkReply *reply);

private:
    void loadModelList();

    Ui::MainWindow *ui;
    std::string modelPath;
    QNetworkAccessManager *networkManager;
};

#endif // MAINWINDOW_H
