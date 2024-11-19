#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Initialize Ollama object
    ollama::setServerURL("http://localhost:11434");

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

    // Set up the chat text edit
    chatTextEdit = new QTextEdit(centralWidget);
    chatTextEdit->setReadOnly(true);
    rightLayout->addWidget(chatTextEdit);

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

    // Add the default workspace
    addWorkspace();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addWorkspace()
{
    static int workspaceCount = 1;
    QString workspaceName = "Workspace " + QString::number(workspaceCount++);
    QListWidgetItem *item = new QListWidgetItem(workspaceName, workspacesList);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    workspacesList->addItem(item);
    workspaceModels[workspaceName] = ""; // Initialize with no model
}

void MainWindow::selectWorkspace(QListWidgetItem *item)
{
    QString workspaceName = item->text();
    chatTextEdit->clear();
    chatTextEdit->append("Selected Workspace: " + workspaceName);

    // Open settings dialog if no model is set for this workspace
    if (workspaceModels[workspaceName].isEmpty()) {
        openSettings(item);
    } else {
        chatTextEdit->append("Model: " + workspaceModels[workspaceName]);
    }
}

void MainWindow::sendMessage()
{
    QString message = inputLineEdit->text();
    if (!message.isEmpty()) {
        // Clear the input line edit
        inputLineEdit->clear();

        // Get the selected workspace
        QListWidgetItem *currentItem = workspacesList->currentItem();
        if (!currentItem) return;
        QString workspaceName = currentItem->text();

        // Get the model associated with the workspace
        QString modelName = workspaceModels[workspaceName];
        if (modelName.isEmpty()) {
            chatTextEdit->append("No model selected for this workspace.");
            return;
        }

        // Create a request to Ollama
        ollama::request request(modelName.toStdString(), message.toStdString());

        // Generate a response from Ollama
        ollama::response response = ollama::generate(request);

        // Display the response in the chat text edit
        if (response.is_valid()) {
            chatTextEdit->append("You: " + message);
            chatTextEdit->append("Ollama: " + QString::fromStdString(response.as_simple_string()));
        } else {
            chatTextEdit->append("Error: " + QString::fromStdString(response.get_error()));
        }
    }
}

void MainWindow::clearChat()
{
    chatTextEdit->clear();
}

void MainWindow::openSettings(QListWidgetItem *item)
{
    QString workspaceName = item->text();

    // Create a dialog for model selection
    QDialog settingsDialog(this);
    settingsDialog.setWindowTitle("Select Model for " + workspaceName);

    QVBoxLayout *layout = new QVBoxLayout(&settingsDialog);
    QComboBox *modelComboBox = new QComboBox(&settingsDialog);
    QPushButton *okButton = new QPushButton("OK", &settingsDialog);

    // Load available models from Ollama
    std::vector<std::string> models = ollama::list_models();
    for (const auto& model : models) {
        modelComboBox->addItem(QString::fromStdString(model));
    }

    layout->addWidget(modelComboBox);
    layout->addWidget(okButton);

    connect(okButton, &QPushButton::clicked, &settingsDialog, &QDialog::accept);

    if (settingsDialog.exec() == QDialog::Accepted) {
        QString selectedModel = modelComboBox->currentText();
        workspaceModels[workspaceName] = selectedModel;
        chatTextEdit->append("Model " + selectedModel + " selected for workspace " + workspaceName);
    }
}

void MainWindow::loadModelList()
{
    // This method is no longer needed since model selection is handled in openSettings
}

void MainWindow::onModelLoaded(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        // Handle the model loaded response
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        // Process the JSON document
    } else {
        // Handle the error
        chatTextEdit->append("Error loading model: " + reply->errorString());
    }
    reply->deleteLater();
}

void MainWindow::onModelListFetched(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        // Handle the model list fetched response
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        // Process the JSON document
    } else {
        // Handle the error
        chatTextEdit->append("Error fetching model list: " + reply->errorString());
    }
    reply->deleteLater();
}

void MainWindow::onInferenceFinished(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        // Handle the inference finished response
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        // Process the JSON document
    } else {
        // Handle the error
        chatTextEdit->append("Error in inference: " + reply->errorString());
    }
    reply->deleteLater();
}

void MainWindow::onInferenceDataReady(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        // Handle the inference data ready response
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        // Process the JSON document
    } else {
        // Handle the error
        chatTextEdit->append("Error in inference data: " + reply->errorString());
    }
    reply->deleteLater();
}
