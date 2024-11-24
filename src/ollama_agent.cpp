// ollama_agent.cpp
#include "ollama_agent.h"
#include "Ollama.h"
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QTextDocument>
#include <sstream>

void OllamaAgent::setServerURL(const std::string& url) {
    serverURL = url;
    ollamaInstance.setServerURL(url);
}

std::vector<std::string> OllamaAgent::list_models() {
    return ollamaInstance.list_models();
}

std::vector<std::string> OllamaAgent::list_running_models() {
    return ollamaInstance.list_running_models();
}

bool OllamaAgent::load_model(const std::string& modelName) {
    return ollamaInstance.load_model(modelName);
}

void OllamaAgent::generate(const std::string& modelName, const std::string& prompt, std::function<void(const std::string&)> callback) {
    std::ostringstream responseStream;
    ollamaInstance.generate(modelName, prompt, [this, callback, &responseStream](const std::string& response) {
        responseStream << response;
        qDebug() << "Received response from model:" << QString::fromStdString(response);

        // Check if the response ends with a period or some other completion signal
        if (response.back() == '.') {
            std::string fullResponse = responseStream.str();
            QString responseText = QString::fromStdString(fullResponse);
            QString htmlText = markdownToHtml(responseText); // Convert Markdown to HTML
            callback(htmlText.toStdString());
            responseStream.str(""); // Clear the stream for the next response
        }
    });
}

QJsonObject OllamaAgent::getSettings() const {
    QJsonObject settings;
    settings["serverURL"] = QString::fromStdString(serverURL);
    return settings;
}

void OllamaAgent::setSettings(const QJsonObject& settings) {
    serverURL = settings["serverURL"].toString().toStdString();
    ollamaInstance.setServerURL(serverURL);
}

// Define the markdownToHtml function
QString OllamaAgent::markdownToHtml(const QString& markdownText) {
    QTextDocument document;
    document.setMarkdown(markdownText);
    return document.toHtml();
}
