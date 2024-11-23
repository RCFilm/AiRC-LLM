// mainwindow.cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QException>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QNetworkRequest>
#include <functional>
#include <vector>
#include <future>
#include <cmark.h>
#include <QRegularExpression> // Add this include

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Initialize network manager
    networkManager = new QNetworkAccessManager(this);

    // Set up the central widget and layout
    centralWidget = new QWidget(this);
    mainLayout = new QHBoxLayout(centralWidget); // Ensure this is QHBoxLayout

    // Create a QSplitter to manage the layout
    QSplitter *splitter = new QSplitter(Qt::Horizontal, centralWidget);
    mainLayout->addWidget(splitter);

    // Declare and initialize leftWidget and rightWidget
    QWidget *leftWidget = new QWidget(splitter);
    QWidget *rightWidget = new QWidget(splitter);

    // Set up the left widget (workspace sidebar)
    leftLayout = new QVBoxLayout(leftWidget);

    // Set up the add workspace button
    addWorkspaceButton = new QPushButton("+", leftWidget);
    addWorkspaceButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    leftLayout->addWidget(addWorkspaceButton);

    // Set up the workspaces list
    workspacesList = new QListWidget(leftWidget);
    workspacesList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    leftLayout->addWidget(workspacesList);

    // Set up the right widget (main chat window)
    rightLayout = new QVBoxLayout(rightWidget);

    // Set up the chat text browser
    chatTextBrowser = new QTextBrowser(rightWidget);
    chatTextBrowser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rightLayout->addWidget(chatTextBrowser);

    // Set up the input line edit and send button
    inputLineEdit = new QLineEdit(rightWidget);
    inputLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sendButton = new QPushButton("Send", rightWidget);
    sendButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    clearButton = new QPushButton("Clear", rightWidget);
    clearButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // Declare and initialize settingsButton
    settingsButton = new QPushButton("Settings", rightWidget);
    settingsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->addWidget(inputLineEdit);
    inputLayout->addWidget(sendButton);
    inputLayout->addWidget(clearButton);
    inputLayout->addWidget(settingsButton);

    rightLayout->addLayout(inputLayout);

    // Add widgets to the splitter
    splitter->addWidget(leftWidget);
    splitter->addWidget(rightWidget);

    // Set initial sizes for the splitter
    QList<int> sizes;
    sizes << 100 << 900; // Adjust these values as needed to achieve the desired ratio
    splitter->setSizes(sizes);

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

    // Connect the settings button to the openSettings slot
    connect(settingsButton, &QPushButton::clicked, this, [this]() { openSettings(); });

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
        inputLineEdit->clear();

        QListWidgetItem *currentItem = workspacesList->currentItem();
        if (!currentItem) return;
        int workspaceId = currentItem->data(Qt::UserRole).toInt();

        QString modelName = workspaceMap[workspaceId]->getModel();
        if (modelName.isEmpty()) {
            chatTextBrowser->append("No model selected for this workspace.");
            return;
        }

        if (!verifyModelStartup(modelName)) {
            chatTextBrowser->append("Model is not ready.");
            return;
        }

        // Display loading indicator
        chatTextBrowser->append("Generating response...");

        // Use std::async for asynchronous generation
        auto future = std::async(std::launch::async, [this, workspaceId, modelName, message]() {
            try {
                QString accumulatedResponse;
                bool responseComplete = false;
                workspaceMap[workspaceId]->getAgent()->generate(modelName.toStdString(), message.toStdString(), [this, workspaceId, &accumulatedResponse, &responseComplete](const std::string& response) {
                    accumulatedResponse += QString::fromStdString(response);
                    if (response.find("<<END>>") != std::string::npos) {
                        responseComplete = true;
                        accumulatedResponse.replace("<<END>>", "");
                    }
                    QMetaObject::invokeMethod(this, [this, accumulatedResponse, responseComplete]() {
                        if (responseComplete) {
                            updateChatWithMarkdown(accumulatedResponse);
                        }
                    });
                });
            } catch (const std::runtime_error& e) {
                QMetaObject::invokeMethod(this, [this, e]() {
                    chatTextBrowser->append("Runtime Error: " + QString::fromStdString(e.what()));
                });
            } catch (const std::exception& e) {
                QMetaObject::invokeMethod(this, [this, e]() {
                    chatTextBrowser->append("Error: " + QString::fromStdString(e.what()));
                });
            }
        });
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
        (void)index; // Suppress unused parameter warning
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

QString MainWindow::markdownToHtml(const QString& markdownText) {
    // Trim leading and trailing whitespace
    QString trimmedText = markdownText.trimmed();

    // Correct markdown syntax
    QString correctedText = trimmedText
                                .replace("*#", "#")
                                .replace("*##", "##")
                                .replace("*###", "###")
                                .replace("*####", "####")
                                .replace("*#####", "#####")
                                .replace("*######", "######")
                                .replace("*-", "-")
                                .replace("*[", "[")
                                .replace("*1.", "1.")
                                .replace("*2.", "2.")
                                .replace("*3.", "3.")
                                .replace("*````code```", "```\ncode\n```")
                                .replace("*~~", "~~");

    // Convert markdown to HTML
    char* html = cmark_markdown_to_html(correctedText.toStdString().c_str(), correctedText.size(), CMARK_OPT_DEFAULT);
    QString htmlText = QString::fromUtf8(html);
    free(html);
    return htmlText;
}

void MainWindow::updateChatWithMarkdown(const QString& markdownText) {
    static QString accumulatedText;
    accumulatedText += markdownText;

    // Check if we have accumulated enough text to process
    QRegularExpression re("[.!?]\\s*");
    if (accumulatedText.contains(re)) {
        QStringList sentences = accumulatedText.split(re, Qt::SkipEmptyParts);
        for (const QString& sentence : sentences) {
            QString htmlText = markdownToHtml(sentence);
            chatTextBrowser->append(htmlText);
        }
        accumulatedText.clear();
    }
}
