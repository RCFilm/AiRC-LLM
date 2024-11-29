// ollama_agent.cpp
#include "ollama_agent.h"
#include "Ollama.hpp"
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QTextDocument>
#include <sstream>
#include <QMetaObject>
#include <QCoreApplication>
#include <future>

OllamaAgent::OllamaAgent() : serverURL("http://localhost:11434") {
    ollama::setServerURL(serverURL);
}

void OllamaAgent::setServerURL(const std::string& url) {
    ollama::setServerURL(url);
    serverURL = url;
}

std::vector<std::string> OllamaAgent::list_models() {
    try {
        return ollama::list_models();
    } catch (const ollama::exception& e) {
        qCritical() << "Error listing models:" << e.what();
        return {};
    }
}

std::vector<std::string> OllamaAgent::list_running_models() {
    try {
        return ollama::list_running_models();
    } catch (const ollama::exception& e) {
        qCritical() << "Error listing running models:" << e.what();
        return {};
    }
}

bool OllamaAgent::load_model(const std::string& modelName) {
    try {
        return ollama::load_model(modelName);
    } catch (const ollama::exception& e) {
        qCritical() << "Error loading model:" << e.what();
        return false;
    }
}

void OllamaAgent::generate(const std::string& modelName, const std::string& prompt, std::function<void(const std::string&)> callback) {
    std::ostringstream responseStream;
    std::future<void> future = std::async(std::launch::async, [this, modelName, prompt, callback, &responseStream]() {
        try {
            ollama::generate(modelName, prompt, [this, callback, &responseStream](const ollama::response& response) {
                responseStream << response.as_simple_string();
                qDebug() << "Received response from model:" << QString::fromStdString(response.as_simple_string());

                if (response.as_json()["done"] == true) {
                    std::string fullResponse = responseStream.str();
                    QString responseText = QString::fromStdString(fullResponse);
                    QString htmlText = markdownToHtml(responseText);

                    QMetaObject::invokeMethod(QCoreApplication::instance(), [callback, htmlText]() {
                        callback(htmlText.toStdString());
                    });

                    responseStream.str("");
                }
            });
        } catch (const ollama::exception& e) {
            qCritical() << "Error generating response:" << e.what();
            QMetaObject::invokeMethod(QCoreApplication::instance(), [callback, e]() {
                callback("Error: " + std::string(e.what()));
            });
        }
    });
}

QJsonObject OllamaAgent::getSettings() const {
    QJsonObject settings;
    settings["serverURL"] = QString::fromStdString(serverURL);
    return settings;
}

void OllamaAgent::setSettings(const QJsonObject& settings) {
    ollama::setServerURL(settings["serverURL"].toString().toStdString());
    serverURL = settings["serverURL"].toString().toStdString();
}

QString OllamaAgent::markdownToHtml(const QString& markdownText) {
    QTextDocument document;
    document.setMarkdown(markdownText);
    return document.toHtml();
}
