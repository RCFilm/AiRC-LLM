#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QException>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QThread>
#include <atomic>
#include <iostream>
#include <thread>
#include <chrono>
#include <functional>
#include <vector>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Initialize network manager
    networkManager = new QNetworkAccessManager(this);

    // Set up the central widget and layout
    centralWidget = new QWidget(this);
    mainLayout = new QHBoxLayout(centralWidget);
    leftLayout = new QVBoxLayout();
    rightLayout = new QVBoxLayout();

    // Set up the add workspace button
    addWorkspaceButton = new QPushButton("+", centralWidget);
    leftLayout->addWidget(addWorkspaceButton);

    // Set up the workspaces list
    workspacesList = new QListWidget(centralWidget);
    leftLayout->addWidget(workspacesList);

    // Set up the chat text browser
    chatTextBrowser = new QTextBrowser(centralWidget);
    rightLayout->addWidget(chatTextBrowser);

    // Set up the input line edit and send button
    inputLineEdit = new QLineEdit(centralWidget);
    sendButton = new QPushButton("Send", centralWidget);
    clearButton = new QPushButton("Clear", centralWidget);
    rightLayout->addWidget(inputLineEdit);
    rightLayout->addWidget(sendButton);
    rightLayout->addWidget(clearButton);

    // Add layouts to the main layout
    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);

    // Set the central widget
    setCentralWidget(centralWidget);

    // Connect the add workspace button to the addWorkspace slot
    connect(addWorkspaceButton, &QPushButton::clicked, this, &MainWindow::addWorkspace);

    // Connect the workspaces list to the selectWorkspace slot
    connect(workspacesList, &QListWidget::itemClicked, this, &MainWindow::selectWorkspace);

    // Connect the send button to the sendMessage slot
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::sendMessage);

    // Connect the clear button to the clearChat slot
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::clearChat);

    // Connect the input line edit to the sendMessage slot when Enter is pressed
    connect(inputLineEdit, &QLineEdit::returnPressed, this, &MainWindow::sendMessage);

    // Load workspaces from file
    loadWorkspaces();

    // Add the default workspace if no workspaces are loaded
    if (workspaceMap.empty()) {
        addWorkspace();
    }

    // Set up context menu for workspaces list
    workspacesList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(workspacesList, &QListWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);

    // Connect double-click to open settings
    connect(workspacesList, &QListWidget::itemDoubleClicked, this, &MainWindow::openSettings);
}

MainWindow::~MainWindow() {
    // Save workspaces to file
    saveWorkspaces();

    delete ui;
    for (auto& pair : workspaceMap) {
        delete pair.second;
    }
}

void MainWindow::addWorkspace() {
    int workspaceId = getNextWorkspaceId();
    QString workspaceName = "Workspace " + QString::number(workspaceId);

    // Ensure the workspace name is unique
    while (workspaceMap.find(workspaceId) != workspaceMap.end()) {
        workspaceId = getNextWorkspaceId();
        workspaceName = "Workspace " + QString::number(workspaceId);
    }

    QListWidgetItem *item = new QListWidgetItem(workspaceName, workspacesList);
    item->setData(Qt::UserRole, workspaceId); // Store the workspace ID in the item's data
    workspacesList->addItem(item);

    // Create an instance of OllamaAgent and pass it to the workspace
    OllamaAgent* ollamaAgent = new OllamaAgent();
    workspaceMap[workspaceId] = new Workspace(workspaceName, workspaceId, ollamaAgent, "Ollama");

    // Open settings dialog to select a model for the new workspace
    openSettings(item);

    // Save workspaces to file after adding a new workspace
    saveWorkspaces();
}

void MainWindow::selectWorkspace(QListWidgetItem *item) {
    if (!item) return; // Ensure the item is valid

    int workspaceId = item->data(Qt::UserRole).toInt();
    QString workspaceName = item->text();
    chatTextBrowser->clear();
    chatTextBrowser->append("Selected Workspace: " + workspaceName);

    // Load chat history for the selected workspace
    if (workspaceMap.find(workspaceId) != workspaceMap.end()) {
        QVector<QString> chatHistory = workspaceMap[workspaceId]->getChatHistory();
        for (const auto& message : chatHistory) {
            chatTextBrowser->append(message);
        }
    }
}

void MainWindow::sendMessage() {
    QString message = inputLineEdit->text();
    if (!message.isEmpty()) {
        // Clear the input line edit
        inputLineEdit->clear();

        // Get the selected workspace
        QListWidgetItem *currentItem = workspacesList->currentItem();
        if (!currentItem) return;
        int workspaceId = currentItem->data(Qt::UserRole).toInt();

        // Get the model associated with the workspace
        QString modelName = workspaceMap[workspaceId]->getModel();
        if (modelName.isEmpty()) {
            chatTextBrowser->append("No model selected for this workspace.");
            return;
        }

        try {
            // Verify model startup
            if (!verifyModelStartup(modelName)) {
                chatTextBrowser->append("Model is not ready.");
                return;
            }

            // Create a request to the LLM API
            workspaceMap[workspaceId]->getAgent()->generate(modelName.toStdString(), message.toStdString(), [this, workspaceId](const std::string& response) {
                // Normalize the response text
                QString normalizedResponse = QString::fromStdString(response).trimmed();
                qDebug() << "Normalized response:" << normalizedResponse;

                // Replace newline characters with spaces to ensure the response appears as a continuous paragraph
                normalizedResponse.replace('\n', ' ');

                if (!normalizedResponse.isEmpty()) {
                    chatTextBrowser->append(normalizedResponse);

                    // Add embedding to Hnswlib index
                    std::vector<float> embedding = workspaceMap[workspaceId]->getEmbedding(normalizedResponse.toStdString());
                    workspaceMap[workspaceId]->addEmbedding(embedding, normalizedResponse);

                    // Save the chat message to the workspace
                    workspaceMap[workspaceId]->addChatMessage("You: " + normalizedResponse);

                    // Save workspaces to file after sending a message
                    saveWorkspaces();
                } else {
                    chatTextBrowser->append("Error: Empty response received.");
                }
            });

            // Save workspaces to file after sending a message
            saveWorkspaces();
        } catch (const std::exception& e) {
            qCritical() << "Error sending message:" << e.what();
            chatTextBrowser->append("Error: " + QString::fromStdString(e.what()));
        } catch (...) {
            qCritical() << "Unknown exception caught";
            chatTextBrowser->append("Error: Unknown exception");
        }
    }
}

void MainWindow::clearChat() {
    chatTextBrowser->clear();
}

void MainWindow::openSettings(QListWidgetItem *item) {
    if (!item) return; // Ensure the item is valid

    int workspaceId = item->data(Qt::UserRole).toInt();
    QString workspaceName = item->text();

    // Create a dialog for model selection
    QDialog settingsDialog(this);
    settingsDialog.setWindowTitle("Settings for " + workspaceName);

    QVBoxLayout *layout = new QVBoxLayout(&settingsDialog);
    QFormLayout *formLayout = new QFormLayout();

    // API selection
    QComboBox *apiComboBox = new QComboBox(&settingsDialog);
    apiComboBox->addItem("Ollama");
    apiComboBox->addItem("HuggingFace");
    formLayout->addRow("API:", apiComboBox);

    // Model selection
    QComboBox *modelComboBox = new QComboBox(&settingsDialog);
    formLayout->addRow("Model:", modelComboBox);

    // Other settings (placeholder)
    QLineEdit *otherSetting1 = new QLineEdit(&settingsDialog);
    formLayout->addRow("Other Setting 1:", otherSetting1);

    QLineEdit *otherSetting2 = new QLineEdit(&settingsDialog);
    formLayout->addRow("Other Setting 2:", otherSetting2);

    layout->addLayout(formLayout);

    QPushButton *okButton = new QPushButton("OK", &settingsDialog);
    layout->addWidget(okButton);

    connect(okButton, &QPushButton::clicked, &settingsDialog, &QDialog::accept);

    // Connect the apiComboBox to update the modelComboBox dynamically
    connect(apiComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        modelComboBox->clear();
        QString selectedApi = apiComboBox->currentText();
        LlmAgentInterface* agent = createAgent(selectedApi);
        if (agent) {
            std::vector<std::string> models = agent->list_models();
            for (const auto& model : models) {
                modelComboBox->addItem(QString::fromStdString(model));
            }
        }
    });

    // Initialize the modelComboBox with the current agent's models
    QString currentApiType = workspaceMap[workspaceId]->getApiType();
    apiComboBox->setCurrentIndex(apiComboBox->findText(currentApiType));
    apiComboBox->currentIndexChanged(apiComboBox->currentIndex());

    if (settingsDialog.exec() == QDialog::Accepted) {
        QString selectedApi = apiComboBox->currentText();
        QString selectedModel = modelComboBox->currentText();
        workspaceMap[workspaceId]->setApiType(selectedApi);
        workspaceMap[workspaceId]->setModel(selectedModel);
        chatTextBrowser->append("API " + selectedApi + " and Model " + selectedModel + " selected for workspace " + workspaceName);

        // Save workspaces to file after setting the model
        saveWorkspaces();
    }
}

void MainWindow::renameWorkspace() {
    QListWidgetItem *currentItem = workspacesList->currentItem();
    if (!currentItem) return;

    int workspaceId = currentItem->data(Qt::UserRole).toInt();
    QString oldName = currentItem->text();

    bool ok;
    QString newName = QInputDialog::getText(this, "Rename Workspace", "New Workspace Name:", QLineEdit::Normal, oldName, &ok);

    if (ok && !newName.isEmpty()) {
        // Ensure the new name is unique
        bool nameExists = false;
        for (int i = 0; i < workspacesList->count(); ++i) {
            if (workspacesList->item(i)->text() == newName) {
                nameExists = true;
                break;
            }
        }

        if (!nameExists) {
            currentItem->setText(newName);
            workspaceMap[workspaceId]->setName(newName);
            saveWorkspaces();
        } else {
            QMessageBox::warning(this, "Rename Workspace", "Workspace name already exists.");
        }
    }
}

void MainWindow::deleteWorkspace() {
    QListWidgetItem *currentItem = workspacesList->currentItem();
    if (!currentItem) return;

    int workspaceId = currentItem->data(Qt::UserRole).toInt();
    QString workspaceName = currentItem->text();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Delete Workspace", "Are you sure you want to delete workspace " + workspaceName + "?", QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        delete workspaceMap[workspaceId];
        workspaceMap.erase(workspaceId);
        delete currentItem;
        saveWorkspaces();
    }
}

void MainWindow::showContextMenu(const QPoint& pos) {
    QMenu contextMenu(tr("Context menu"), this);

    QAction renameAction("Rename Workspace", this);
    QAction deleteAction("Delete Workspace", this);
    QAction settingsAction("Settings", this);

    connect(&renameAction, &QAction::triggered, this, &MainWindow::renameWorkspace);
    connect(&deleteAction, &QAction::triggered, this, &MainWindow::deleteWorkspace);
    connect(&settingsAction, &QAction::triggered, this, [this, pos]() {
        QListWidgetItem* item = workspacesList->itemAt(pos);
        if (item) {
            openSettings(item);
        }
    });

    contextMenu.addAction(&renameAction);
    contextMenu.addAction(&deleteAction);
    contextMenu.addAction(&settingsAction);

    contextMenu.exec(workspacesList->mapToGlobal(pos));
}

void MainWindow::loadWorkspaces() {
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

void MainWindow::saveWorkspaces() {
    try {
        QString filePath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/workspaces.json";
        QFile file(filePath);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qCritical() << "Failed to open workspaces file for writing:" << file.errorString();
            return;
        }

        QJsonArray jsonArray;
        for (const auto& pair : workspaceMap) {
            QJsonObject jsonObject = pair.second->toJson();
            jsonObject["agentType"] = QString::fromStdString(pair.second->getAgent()->getAgentType()); // Use getAgentType()
            jsonObject["apiType"] = pair.second->getApiType();
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

bool MainWindow::verifyModelStartup(const QString& modelName) {
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

        // If the model is not found in the list of running models, ask the user to load the model
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Model Not Running", "The model " + modelName + " is not running. Do you want to load it?", QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            if (loadModel(modelName)) {
                modelStatusMap[modelKey] = true;
                return true;
            }
        }

        modelStatusMap[modelKey] = false;
        return false;
    } catch (const ollama::exception& e) {
        qCritical() << "Error verifying model startup:" << e.what();
        QMessageBox::critical(this, "Model Startup Verification Error", "Error verifying model startup: " + modelName + "\nDetails: " + QString::fromStdString(e.what()));
        modelStatusMap[modelKey] = false;
        return false;
    } catch (const std::exception& e) {
        qCritical() << "Standard exception caught in verifyModelStartup:" << e.what();
        QMessageBox::critical(this, "Model Startup Verification Error", "Standard exception caught while verifying model startup: " + modelName + "\nDetails: " + QString::fromStdString(e.what()));
        modelStatusMap[modelKey] = false;
        return false;
    } catch (...) {
        qCritical() << "Unknown exception caught in verifyModelStartup";
        QMessageBox::critical(this, "Model Startup Verification Error", "Unknown exception caught while verifying model startup: " + modelName);
        modelStatusMap[modelKey] = false;
        return false;
    }
}

bool MainWindow::loadModel(const QString& modelName) {
    try {
        qDebug() << "Loading model:" << modelName;
        bool modelLoaded = ollama::load_model(modelName.toStdString());
        if (modelLoaded) {
            qDebug() << "Model loaded successfully:" << modelName;
        } else {
            qDebug() << "Failed to load model:" << modelName;
            QMessageBox::critical(this, "Model Loading Error", "Failed to load model: " + modelName + "\nPlease check the server and try again.");
        }
        return modelLoaded;
    } catch (const ollama::exception& e) {
        qCritical() << "Error loading model:" << e.what();
        QMessageBox::critical(this, "Model Loading Error", "Error loading model: " + modelName + "\nDetails: " + QString::fromStdString(e.what()));
        return false;
    } catch (const std::exception& e) {
        qCritical() << "Standard exception caught in loadModel:" << e.what();
        QMessageBox::critical(this, "Model Loading Error", "Standard exception caught while loading model: " + modelName + "\nDetails: " + QString::fromStdString(e.what()));
        return false;
    } catch (...) {
        qCritical() << "Unknown exception caught in loadModel";
        QMessageBox::critical(this, "Model Loading Error", "Unknown exception caught while loading model: " + modelName);
        return false;
    }
}

int MainWindow::getNextWorkspaceId() const {
    int maxId = 0;
    for (const auto& pair : workspaceMap) {
        if (pair.first > maxId) {
            maxId = pair.first;
        }
    }
    return maxId + 1;
}

LlmAgentInterface* MainWindow::createAgent(const QString& apiType) {
    if (apiType == "Ollama") {
        return new OllamaAgent();
    } else if (apiType == "HuggingFace") {
        return new HuggingFaceAgent();
    }
    return nullptr;
}
