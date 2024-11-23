// ollama_agent.cpp
#include "ollama_agent.h"
#include "Ollama.h"
#include <QDebug>
#include <algorithm>

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
    ollamaInstance.generate(modelName, prompt, [this, callback](const std::string& response) {
        std::string trimmedResponse = trim(response); // Trim the response
        qDebug() << "Received response from model:" << QString::fromStdString(trimmedResponse);
        callback(trimmedResponse);
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

// Define the trim function
std::string OllamaAgent::trim(const std::string& str) {
    size_t first = str.find_first_not_of(' ');
    if (std::string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}
