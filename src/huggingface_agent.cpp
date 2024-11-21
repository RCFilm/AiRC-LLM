// huggingface_agent.cpp
#include "huggingface_agent.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

void HuggingFaceAgent::setServerURL(const std::string& url) {
    serverURL = url;
}

std::vector<std::string> HuggingFaceAgent::list_models() {
    // Implement the logic to list models from Hugging Face API
    // This is a placeholder implementation
    return {"model1", "model2", "model3"};
}

std::vector<std::string> HuggingFaceAgent::list_running_models() {
    // Implement the logic to list running models from Hugging Face API
    // This is a placeholder implementation
    return {"model1", "model2"};
}

bool HuggingFaceAgent::load_model(const std::string& /*modelName*/) {
    // Implement the logic to load a model from Hugging Face API
    // This is a placeholder implementation
    return true;
}

void HuggingFaceAgent::generate(const std::string& /*modelName*/, const std::string& prompt, std::function<void(const std::string&)> callback) {
    // Implement the logic to generate a response from Hugging Face API
    // This is a placeholder implementation
    std::string response = "Generated response for " + prompt;
    if (!response.empty()) {
        callback(response);
    } else {
        callback("Error: Empty response received.");
    }
}

QJsonObject HuggingFaceAgent::getSettings() const {
    QJsonObject settings;
    settings["serverURL"] = QString::fromStdString(serverURL);
    settings["apiKey"] = QString::fromStdString(apiKey);
    return settings;
}

void HuggingFaceAgent::setSettings(const QJsonObject& settings) {
    serverURL = settings["serverURL"].toString().toStdString();
    apiKey = settings["apiKey"].toString().toStdString();
}
