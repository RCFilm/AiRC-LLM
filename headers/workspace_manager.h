// workspace_manager.h
#ifndef WORKSPACE_MANAGER_H
#define WORKSPACE_MANAGER_H

#include <QObject>
#include <QMap>
#include "workspace.h"

class WorkspaceManager : public QObject {
    Q_OBJECT

public:
    explicit WorkspaceManager(QObject *parent = nullptr);
    ~WorkspaceManager();

    void addWorkspace(const QString& name, LlmAgentInterface* agent, const QString& apiType);
    void removeWorkspace(int id);
    Workspace* getWorkspace(int id);
    QMap<int, Workspace*> getWorkspaces() const;

private:
    QMap<int, Workspace*> workspaceMap;
    int nextWorkspaceId;
};

#endif // WORKSPACE_MANAGER_H
