#ifndef MAINWINDOW_HELPERS_H
#define MAINWINDOW_HELPERS_H

#include <QString>
#include <QJsonObject>
#include <QListWidgetItem>
#include <QTextBrowser>  // Include QTextBrowser
#include <QException>    // Include QException
#include <QMessageBox>   // Include QMessageBox
#include "workspace.h"

namespace MainWindowHelpers {
void updateChatWithMarkdown(QTextBrowser* chatTextBrowser, const QString& markdownText);
LlmAgentInterface* createAgent(const QString& apiType);
void loadWorkspaces(QMap<int, Workspace*>& workspaceMap, QListWidget* workspacesList);
void saveWorkspaces(const QMap<int, Workspace*>& workspaceMap);
bool verifyModelStartup(const QString& modelName, std::unordered_map<std::string, bool>& modelStatusMap);
bool loadModel(const QString& modelName);
int getNextWorkspaceId(const QMap<int, Workspace*>& workspaceMap);
}

#endif // MAINWINDOW_HELPERS_H
