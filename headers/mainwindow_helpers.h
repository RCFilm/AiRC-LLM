// mainwindow_helpers.h
#ifndef MAINWINDOW_HELPERS_H
#define MAINWINDOW_HELPERS_H

#include <QString>
#include <QTextBrowser>
#include <QMap>
#include <QListWidget>
#include <unordered_map>
#include "workspace.h"
#include "debugwindow.h" // Include the DebugWindow header

namespace MainWindowHelpers {

void updateChatWithMarkdown(QTextBrowser* chatTextBrowser, const QString& markdownText);
LlmAgentInterface* createAgent(const QString& apiType, DebugWindow* debugWindow);
void loadWorkspaces(QMap<int, Workspace*>& workspaceMap, QListWidget* workspacesList, DebugWindow* debugWindow);
void saveWorkspaces(const QMap<int, Workspace*>& workspaceMap);
bool verifyModelStartup(const QString& modelName, std::unordered_map<std::string, bool>& modelStatusMap);
bool loadModel(const QString& modelName);
int getNextWorkspaceId(const QMap<int, Workspace*>& workspaceMap);

}

#endif // MAINWINDOW_HELPERS_H
