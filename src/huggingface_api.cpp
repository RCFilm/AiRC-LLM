// huggingface_api.cpp
#include "huggingface_api.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

void HuggingFaceApi::setServerURL(const std::string& url) {
    serverURL = url;
}

std::vector<std::string> HuggingFaceApi::list_models() {
    // Implement the logic to list models from Hugging Face API
    // This is a placeholder implementation
    return {"model1", "model2", "model3"};
}

std::vector<std::string> HuggingFaceApi::list_running_models() {
    // Implement the logic to list running models from Hugging Face API
    // This is a placeholder implementation
    return {"model1", "model2"};
}

bool HuggingFaceApi::load_model(const std::string& _modelName) {
    // Implement the logic to load a model from Hugging Face API
    // This is a placeholder implementation
    return true;
}

void HuggingFaceApi::generate(const std::string& _modelName, const std::string& prompt, std::function<void(const std::string&)> callback) {
    // Implement the logic to generate a response from Hugging Face API
    // This is a placeholder implementation
    std::string response = "Generated response for " + prompt;
    callback(response);
}
