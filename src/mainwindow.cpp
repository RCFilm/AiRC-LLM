// mainwindow.cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mainwindow_helpers.h"
#include <QException>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QNetworkRequest>
#include <functional>
#include <vector>
#include <future>
#include <QRegularExpression>
#include <QTimer>
#include <QMessageBox>

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
    // Set the background color to a lighter gray
    chatTextBrowser->setStyleSheet("QTextBrowser { background-color: rgb(50, 51, 61); }");
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
    for (auto it = workspaceMap.begin(); it != workspaceMap.end(); ++it) {
        delete it->second;
    }
}

void MainWindow::loadWorkspaces() {
    QMap<int, Workspace*> qmapWorkspaceMap;
    for (auto it = workspaceMap.begin(); it != workspaceMap.end(); ++it) {
        qmapWorkspaceMap[it->first] = it->second;
    }
    MainWindowHelpers::loadWorkspaces(qmapWorkspaceMap, workspacesList);
    workspaceMap = qmapWorkspaceMap.toStdMap();
}

void MainWindow::saveWorkspaces() {
    QMap<int, Workspace*> qmapWorkspaceMap;
    for (auto it = workspaceMap.begin(); it != workspaceMap.end(); ++it) {
        qmapWorkspaceMap[it->first] = it->second;
    }
    MainWindowHelpers::saveWorkspaces(qmapWorkspaceMap);
}

void MainWindow::addWorkspace() {
    QMap<int, Workspace*> qmapWorkspaceMap;
    for (auto it = workspaceMap.begin(); it != workspaceMap.end(); ++it) {
        qmapWorkspaceMap[it->first] = it->second;
    }
    int workspaceId = MainWindowHelpers::getNextWorkspaceId(qmapWorkspaceMap);
    QString workspaceName = "Workspace " + QString::number(workspaceId);

    // Ensure the workspace name is unique
    while (workspaceMap.find(workspaceId) != workspaceMap.end()) {
        workspaceId = MainWindowHelpers::getNextWorkspaceId(qmapWorkspaceMap);
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

    // Select the newly created workspace
    workspacesList->setCurrentItem(item);
    selectWorkspace(item);

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

        if (!MainWindowHelpers::verifyModelStartup(modelName, modelStatusMap)) {
            chatTextBrowser->append("Model is not ready.");
            return;
        }

        // Display loading indicator
        chatTextBrowser->append("Generating response...");

        // Use std::async for asynchronous generation
        auto future = std::async(std::launch::async, [this, workspaceId, modelName, message]() {
            try {
                workspaceMap[workspaceId]->getAgent()->generate(modelName.toStdString(), message.toStdString(), [this, workspaceId](const std::string& response) {
                    QString responseText = QString::fromStdString(response);
                    QMetaObject::invokeMethod(this, [this, responseText, workspaceId]() {
                        MainWindowHelpers::updateChatWithMarkdown(chatTextBrowser, responseText);
                        workspaceMap[workspaceId]->addChatMessage(responseText);
                        saveWorkspaces(); // Save workspaces after adding a chat message
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
        LlmAgentInterface* agent = MainWindowHelpers::createAgent(selectedApi);
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

void MainWindow::deleteAllWorkspaces() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Delete All Workspaces", "Are you sure you want to delete all workspaces?", QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        for (auto it = workspaceMap.begin(); it != workspaceMap.end(); ++it) {
            delete it->second;
        }
        workspaceMap.clear();
        workspacesList->clear();
        saveWorkspaces();
    }
}

void MainWindow::showContextMenu(const QPoint& pos) {
    QMenu contextMenu(tr("Context menu"), this);

    QAction renameAction("Rename Workspace", this);
    QAction deleteAction("Delete Workspace", this);
    QAction settingsAction("Settings", this);
    QAction deleteAllAction("Delete All Workspaces", this);

    connect(&renameAction, &QAction::triggered, this, &MainWindow::renameWorkspace);
    connect(&deleteAction, &QAction::triggered, this, &MainWindow::deleteWorkspace);
    connect(&settingsAction, &QAction::triggered, this, [this, pos]() {
        QListWidgetItem* item = workspacesList->itemAt(pos);
        if (item) {
            openSettings(item);
        }
    });
    connect(&deleteAllAction, &QAction::triggered, this, &MainWindow::deleteAllWorkspaces);

    contextMenu.addAction(&renameAction);
    contextMenu.addAction(&deleteAction);
    contextMenu.addAction(&settingsAction);
    contextMenu.addAction(&deleteAllAction);

    contextMenu.exec(workspacesList->mapToGlobal(pos));
}
