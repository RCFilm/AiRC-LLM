#include "mainwindow_helpers.h"
#include "ollama_agent.h"
#include "huggingface_agent.h"
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QNetworkRequest>
#include <functional>
#include <vector>
#include <future>
#include <QRegularExpression>

namespace MainWindowHelpers {

void updateChatWithMarkdown(QTextBrowser* chatTextBrowser, const QString& markdownText) {
    chatTextBrowser->append(markdownText);
}

LlmAgentInterface* createAgent(const QString& apiType) {
    if (apiType == "Ollama") {
        return new OllamaAgent();
    } else if (apiType == "HuggingFace") {
        return new HuggingFaceAgent();
    }
    return nullptr;
}

void loadWorkspaces(QMap<int, Workspace*>& workspaceMap, QListWidget* workspacesList) {
    try {
        QString filePath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/workspaces.json";
        QFile file(filePath);

        if (!file.exists()) {
            QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
            if (!dir.mkpath(".")) {
                qCritical() << "Failed to create directory for workspaces file.";
                return;
            }

            // Create an empty file if it doesn't exist
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                qCritical() << "Failed to create workspaces file:" << file.errorString();
                return;
            }
            file.close();
        }

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qCritical() << "Failed to open workspaces file for reading:" << file.errorString();
            return;
        }

        QByteArray data = file.readAll();
        file.close();

        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        if (jsonDoc.isNull() || !jsonDoc.isArray()) {
            qCritical() << "Invalid JSON format in workspaces file.";
            return;
        }

        QJsonArray jsonArray = jsonDoc.array();

        for (const auto& jsonValue : jsonArray) {
            if (jsonValue.isObject()) {
                QJsonObject jsonObject = jsonValue.toObject();
                QString agentType = jsonObject["agentType"].toString();
                QString apiType = jsonObject["apiType"].toString();
                LlmAgentInterface* agent = createAgent(apiType);
                if (agent) {
                    Workspace* workspace = new Workspace(Workspace::fromJson(jsonObject, agent));
                    workspaceMap[workspace->getId()] = workspace;
                    QListWidgetItem *item = new QListWidgetItem(workspace->getName(), workspacesList);
                    item->setData(Qt::UserRole, workspace->getId()); // Store the workspace ID in the item's data
                    workspacesList->addItem(item);
                }
            }
        }
    } catch (const QException& e) {
        qCritical() << "Exception caught in loadWorkspaces:" << e.what();
    } catch (...) {
        qCritical() << "Unknown exception caught in loadWorkspaces";
    }
}

void saveWorkspaces(const QMap<int, Workspace*>& workspaceMap) {
    try {
        QString filePath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/workspaces.json";
        QFile file(filePath);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qCritical() << "Failed to open workspaces file for writing:" << file.errorString();
            return;
        }

        QJsonArray jsonArray;
        for (auto it = workspaceMap.begin(); it != workspaceMap.end(); ++it) {
            QJsonObject jsonObject = it.value()->toJson();
            jsonObject["agentType"] = QString::fromStdString(it.value()->getAgent()->getAgentType()); // Use getAgentType()
            jsonObject["apiType"] = it.value()->getApiType();
            jsonArray.append(jsonObject);
        }

        QJsonDocument jsonDoc(jsonArray);
        file.write(jsonDoc.toJson());
        file.close();
    } catch (const QException& e) {
        qCritical() << "Exception caught in saveWorkspaces:" << e.what();
    } catch (...) {
        qCritical() << "Unknown exception caught in saveWorkspaces";
    }
}

bool verifyModelStartup(const QString& modelName, std::unordered_map<std::string, bool>& modelStatusMap) {
    std::string modelKey = modelName.toStdString();

    // Check if the model status is already known
    if (modelStatusMap.find(modelKey) != modelStatusMap.end()) {
        return modelStatusMap[modelKey];
    }

    try {
        // List the models available locally in the ollama server
        std::vector<std::string> runningModels = ollama::list_running_models();

        // Check if the specified model is in the list of running models
        for (const auto& model : runningModels) {
            if (model == modelKey) {
                modelStatusMap[modelKey] = true;
                return true;
            }
        }

        // If the model is not found in the list of running models, load the model
        if (loadModel(modelName)) {
            modelStatusMap[modelKey] = true;
            return true;
        }

        modelStatusMap[modelKey] = false;
        return false;
    } catch (const ollama::exception& e) {
        qCritical() << "Error verifying model startup:" << e.what();
        QMessageBox::critical(nullptr, "Model Startup Verification Error", "Error verifying model startup: " + modelName + "\nDetails: " + QString::fromStdString(e.what()));
        modelStatusMap[modelKey] = false;
        return false;
    } catch (const std::exception& e) {
        qCritical() << "Standard exception caught in verifyModelStartup:" << e.what();
        QMessageBox::critical(nullptr, "Model Startup Verification Error", "Standard exception caught while verifying model startup: " + modelName + "\nDetails: " + QString::fromStdString(e.what()));
        modelStatusMap[modelKey] = false;
        return false;
    } catch (...) {
        qCritical() << "Unknown exception caught in verifyModelStartup";
        QMessageBox::critical(nullptr, "Model Startup Verification Error", "Unknown exception caught while verifying model startup: " + modelName);
        modelStatusMap[modelKey] = false;
        return false;
    }
}

bool loadModel(const QString& modelName) {
    try {
        qDebug() << "Loading model:" << modelName;
        bool modelLoaded = ollama::load_model(modelName.toStdString());
        if (modelLoaded) {
            qDebug() << "Model loaded successfully:" << modelName;
        } else {
            qDebug() << "Failed to load model:" << modelName;
            QMessageBox::critical(nullptr, "Model Loading Error", "Failed to load model: " + modelName + "\nPlease check the server and try again.");
        }
        return modelLoaded;
    } catch (const ollama::exception& e) {
        qCritical() << "Error loading model:" << e.what();
        QMessageBox::critical(nullptr, "Model Loading Error", "Error loading model: " + modelName + "\nDetails: " + QString::fromStdString(e.what()));
        return false;
    } catch (const std::exception& e) {
        qCritical() << "Standard exception caught in loadModel:" << e.what();
        QMessageBox::critical(nullptr, "Model Loading Error", "Standard exception caught while loading model: " + modelName + "\nDetails: " + QString::fromStdString(e.what()));
        return false;
    } catch (...) {
        qCritical() << "Unknown exception caught in loadModel";
        QMessageBox::critical(nullptr, "Model Loading Error", "Unknown exception caught while loading model: " + modelName);
        return false;
    }
}

int getNextWorkspaceId(const QMap<int, Workspace*>& workspaceMap) {
    int maxId = 0;
    for (auto it = workspaceMap.begin(); it != workspaceMap.end(); ++it) {
        if (it.key() > maxId) {
            maxId = it.key();
        }
    }
    return maxId + 1;
}
}
