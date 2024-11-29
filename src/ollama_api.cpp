// ollama_api.cpp
#include "ollama_api.h"
#include "Ollama.hpp"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>
#include <QDebug>

namespace ollama {

OllamaApi::OllamaApi() : networkManager(new QNetworkAccessManager()) {
    serverURL = "http://localhost:11434";
}

OllamaApi::~OllamaApi() {
    delete networkManager;
}

void OllamaApi::setServerURL(const std::string& url) {
    serverURL = url;
    qDebug() << "Server URL set to:" << QString::fromStdString(serverURL);
}

std::vector<std::string> OllamaApi::list_models() {
    QNetworkRequest request(QUrl(QString::fromStdString(serverURL + "/models")));
    QNetworkReply* reply = networkManager->get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << "Network error:" << reply->errorString();
        throw std::runtime_error("Network error: " + reply->errorString().toStdString());
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll());
    if (jsonDoc.isNull() || !jsonDoc.isArray()) {
        qCritical() << "Invalid JSON response: Expected an array";
        throw std::runtime_error("Invalid JSON response: Expected an array");
    }

    QJsonArray jsonArray = jsonDoc.array();
    std::vector<std::string> models;
    for (const auto& model : jsonArray) {
        if (model.isString()) {
            models.push_back(model.toString().toStdString());
        }
    }

    return models;
}

std::vector<std::string> OllamaApi::list_running_models() {
    QNetworkRequest request(QUrl(QString::fromStdString(serverURL + "/running_models")));
    QNetworkReply* reply = networkManager->get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << "Network error:" << reply->errorString();
        throw std::runtime_error("Network error: " + reply->errorString().toStdString());
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll());
    if (jsonDoc.isNull() || !jsonDoc.isArray()) {
        qCritical() << "Invalid JSON response: Expected an array";
        throw std::runtime_error("Invalid JSON response: Expected an array");
    }

    QJsonArray jsonArray = jsonDoc.array();
    std::vector<std::string> models;
    for (const auto& model : jsonArray) {
        if (model.isString()) {
            models.push_back(model.toString().toStdString());
        }
    }

    return models;
}

bool OllamaApi::load_model(const std::string& modelName) {
    QNetworkRequest request(QUrl(QString::fromStdString(serverURL + "/load_model")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["model"] = QString::fromStdString(modelName);
    QJsonDocument jsonDoc(json);

    QNetworkReply* reply = networkManager->post(request, jsonDoc.toJson());

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << "Network error:" << reply->errorString();
        throw std::runtime_error("Network error: " + reply->errorString().toStdString());
    }

    QJsonDocument responseDoc = QJsonDocument::fromJson(reply->readAll());
    if (responseDoc.isNull() || !responseDoc.isObject()) {
        qCritical() << "Invalid JSON response: Expected an object";
        throw std::runtime_error("Invalid JSON response: Expected an object");
    }

    QJsonObject responseObj = responseDoc.object();
    if (!responseObj.contains("success") || !responseObj["success"].isBool()) {
        qCritical() << "Invalid JSON response: Missing or invalid 'success' field";
        throw std::runtime_error("Invalid JSON response: Missing or invalid 'success' field");
    }

    return responseObj["success"].toBool();
}

void OllamaApi::generate(const std::string& modelName, const std::string& prompt, std::function<void(const std::string&)> callback) {
    generateWithEmbedding(modelName, prompt, callback, true);
}

void OllamaApi::generateWithEmbedding(const std::string& modelName, const std::string& prompt, std::function<void(const std::string&)> callback, bool useEmbedding) {
    if (useEmbedding) {
        QNetworkRequest request(QUrl(QString::fromStdString(serverURL + "/generate")));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QJsonObject json;
        json["model"] = QString::fromStdString(modelName);
        json["prompt"] = QString::fromStdString(prompt);
        QJsonDocument jsonDoc(json);

        QNetworkReply* reply = networkManager->post(request, jsonDoc.toJson());

        QObject::connect(reply, &QNetworkReply::readyRead, [reply, callback]() {
            QByteArray data = reply->readAll();
            QJsonDocument responseDoc = QJsonDocument::fromJson(data);
            if (!responseDoc.isNull() && responseDoc.isObject()) {
                QJsonObject responseObj = responseDoc.object();
                if (responseObj.contains("embedding") && responseObj["embedding"].isString()) {
                    callback(responseObj["embedding"].toString().toStdString());
                }
            }
        });

        QObject::connect(reply, &QNetworkReply::finished, [reply]() {
            reply->deleteLater();
        });
    } else {
        callback("Embedding generation is disabled.");
    }
}

std::string OllamaApi::getServerURL() {
    OllamaApi api;
    return api.serverURL;
}

} // namespace ollama
