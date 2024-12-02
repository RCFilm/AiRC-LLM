// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Include necessary headers
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextBrowser>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QDialog>
#include <QFormLayout>
#include <QComboBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QMenu>
#include <QTimer>
#include <unordered_map>
#include "workspace.h"
#include "huggingface_agent.h"
#include "ollama_agent.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addWorkspace();
    void selectWorkspace(QListWidgetItem *item);
    void sendMessage();
    void clearChat();
    void openSettings(QListWidgetItem *item = nullptr);
    void renameWorkspace();
    void deleteWorkspace();
    void showContextMenu(const QPoint& pos);
    void deleteAllWorkspaces(); // Declare the deleteAllWorkspaces method
    void editUrlAndRepollModels(); // Declare the editUrlAndRepollModels method
    void repollModels(); // Declare the repollModels method
    void streamSendMessage(); // Declare the streamSendMessage method

private:
    Ui::MainWindow *ui;
    QWidget *centralWidget;
    QHBoxLayout *mainLayout;
    QVBoxLayout *leftLayout;
    QVBoxLayout *rightLayout;
    QPushButton *addWorkspaceButton;
    QListWidget *workspacesList;
    QTextBrowser *chatTextBrowser;
    QLineEdit *inputLineEdit;
    QPushButton *sendButton;
    QPushButton *clearButton;
    QPushButton *settingsButton; // Declare settingsButton
    std::map<int, Workspace*> workspaceMap;
    QNetworkAccessManager *networkManager;
    std::unordered_map<std::string, bool> modelStatusMap;

    void loadWorkspaces();
    void saveWorkspaces();
    bool verifyModelStartup(const QString& modelName);
    bool loadModel(const QString& modelName);
    int getNextWorkspaceId() const;
    LlmAgentInterface* createAgent(const QString& apiType);

    // Declare markdown handling functions
    QString markdownToHtml(const QString& markdownText);
    void updateChatWithMarkdown(const QString& markdownText);
    std::vector<float> parseEmbedding(const std::string& embeddingStr); // Declare the parseEmbedding method
};

#endif // MAINWINDOW_H
