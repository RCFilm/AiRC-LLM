#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QVector>
#include <QStringList>
#include <sstream>
#include <QUrl>
#include <QHttpMultiPart>
#include <QIODevice>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QColor>
#include "MarkdownHighlighter.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , networkManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);

    // Apply custom styles
    setStyleSheet(
        "QMainWindow { background-color: #f0f0f0; }"
        "QGroupBox { border: 1px solid #dcdcdc; border-radius: 5px; margin-top: 1ex; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 3px 0 3px; color: #333; }"
        "QPushButton { background-color: #4CAF50; color: white; border: none; border-radius: 5px; padding: 10px 20px; }"
        "QPushButton:hover { background-color: #45a049; }"
        "QLineEdit { border: 1px solid #dcdcdc; border-radius: 5px; padding: 5px; }"
        "QTextEdit { border: 1px solid #dcdcdc; border-radius: 5px; padding: 5px; background-color: white; color: #000; }" // Set text color to black
        "QLabel { color: #333; }"
        );

    // Connect signals and slots
    connect(ui->inputField, &QLineEdit::returnPressed, this, &MainWindow::sendMessage);
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::sendMessage);

    // Load the model list at the start of the application
    loadModelList();

    // Set up the markdown highlighter
    MarkdownHighlighter *highlighter = new MarkdownHighlighter(ui->resultsDisplay->document());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadModelList()
{
    // Fetch the list of models from the API using the /api/tags endpoint
    QString apiEndpoint = ui->apiEndpointField->text();
    QUrl url(QString("%1/api/tags").arg(apiEndpoint));
    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager->get(request);

    // Use a lambda function to pass the reply to the slot
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        onModelListFetched(reply);
    });
}

void MainWindow::onModelListFetched(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        qDebug() << "Response Data:" << responseData;

        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        if (jsonDoc.isNull()) {
            QMessageBox::warning(this, tr("Error"), tr("Failed to parse JSON response!"));
            reply->deleteLater();
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();
        if (jsonObj.contains("models") && jsonObj["models"].isArray()) {
            QJsonArray modelsArray = jsonObj["models"].toArray();
            ui->modelComboBox->clear();
            if (modelsArray.isEmpty()) {
                QMessageBox::information(this, tr("No Models"), tr("No models available."));
            } else {
                for (const QJsonValue &modelValue : modelsArray) {
                    if (modelValue.isObject()) {
                        QJsonObject modelObj = modelValue.toObject();
                        if (modelObj.contains("name") && modelObj["name"].isString()) {
                            QString modelName = modelObj["name"].toString();
                            ui->modelComboBox->addItem(modelName);
                        }
                    }
                }
            }
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Failed to parse model list response!"));
        }
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to fetch model list: %1\nPlease start Ollama.").arg(reply->errorString()));
    }
    reply->deleteLater();
}

void MainWindow::sendMessage()
{
    QString inputStr = ui->inputField->text();
    if (inputStr.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Input data is empty!"));
        return;
    }

    QString selectedModel = ui->modelComboBox->currentText();
    if (selectedModel.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("No model selected!"));
        return;
    }

    // Add the sent message to the results display
    QTextCursor cursor = ui->resultsDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    QTextCharFormat sentFormat;
    cursor.insertText("Sent: ", sentFormat);
    cursor.insertText(inputStr + "\n");
    ui->resultsDisplay->setTextCursor(cursor);

    QString apiEndpoint = ui->apiEndpointField->text();
    QUrl url(QString("%1/api/generate").arg(apiEndpoint));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject requestData;
    requestData["model"] = selectedModel;
    requestData["prompt"] = inputStr;

    QJsonDocument requestDoc(requestData);
    QByteArray requestJsonData = requestDoc.toJson();

    QNetworkReply *reply = networkManager->post(request, requestJsonData);

    // Use a lambda function to pass the reply to the slot
    connect(reply, &QNetworkReply::readyRead, [this, reply]() {
        onInferenceDataReady(reply);
    });

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        onInferenceFinished(reply);
    });
}

void MainWindow::onInferenceDataReady(QNetworkReply *reply)
{
    static bool firstChunk = true;
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    if (jsonDoc.isNull()) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to parse JSON response!"));
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    if (jsonObj.contains("response") && jsonObj["response"].isString()) {
        QString response = jsonObj["response"].toString();
        QTextCursor cursor = ui->resultsDisplay->textCursor();
        cursor.movePosition(QTextCursor::End);
        if (firstChunk) {
            QTextCharFormat responseFormat;
            cursor.insertText("Response: ", responseFormat);
            firstChunk = false;
        }
        cursor.insertText(response);
        ui->resultsDisplay->setTextCursor(cursor);
    }
}

void MainWindow::onInferenceFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        QJsonObject jsonObj = jsonDoc.object();
        if (jsonObj.contains("done") && jsonObj["done"].toBool()) {
            // No need for a message if everything is fine
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Failed to parse inference response!"));
        }
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to run inference: %1").arg(reply->errorString()));
    }
    reply->deleteLater();
}

void MainWindow::onModelLoaded(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        QJsonObject jsonObj = jsonDoc.object();
        if (jsonObj.contains("model") && jsonObj["model"].isString()) {
            QString modelName = jsonObj["model"].toString();
            ui->modelPathLabel->setText(modelName);
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Failed to parse model response!"));
        }
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to load model: %1").arg(reply->errorString()));
    }
    reply->deleteLater();
}
