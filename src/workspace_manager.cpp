// workspace_manager.cpp
#include "workspace_manager.h"

WorkspaceManager::WorkspaceManager(QObject *parent)
    : QObject(parent), nextWorkspaceId(1) {}

WorkspaceManager::~WorkspaceManager() {
    for (auto& pair : workspaceMap) {
        delete pair; // Corrected: Use 'delete' on the pointer
    }
}

void WorkspaceManager::addWorkspace(const QString& name, LlmAgentInterface* agent, const QString& apiType) {
    int id = nextWorkspaceId++;
    workspaceMap[id] = new Workspace(name, id, agent, apiType);
}

void WorkspaceManager::removeWorkspace(int id) {
    delete workspaceMap.take(id);
}

Workspace* WorkspaceManager::getWorkspace(int id) {
    return workspaceMap.value(id, nullptr);
}

QMap<int, Workspace*> WorkspaceManager::getWorkspaces() const {
    return workspaceMap;
}
