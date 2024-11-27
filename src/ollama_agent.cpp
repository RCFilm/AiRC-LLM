// ollama_agent.cpp
#include "ollama_agent.h"
#include "Ollama.h"
#include <QString>
#include <QStringList>
#include <QTextDocument>
#include <sstream>
#include <QMetaObject>
#include <QCoreApplication>
#include <future>
#include "debugwindow.h" // Include the DebugWindow header

void OllamaAgent::setServerURL(const std::string& url) {
    serverURL = url;
    ollamaInstance.setServerURL(url);
}

std::vector<std::string> OllamaAgent::list_models() {
    debugWindow->logMessage("Listing models");
    return ollamaInstance.list_models();
}

std::vector<std::string> OllamaAgent::list_running_models() {
    debugWindow->logMessage("Listing running models");
    return ollamaInstance.list_running_models();
}

bool OllamaAgent::load_model(const std::string& modelName) {
    debugWindow->logMessage("Loading model: " + QString::fromStdString(modelName));
    bool modelLoaded = ollamaInstance.load_model(modelName);
    if (modelLoaded) {
        debugWindow->logMessage("Model loaded successfully: " + QString::fromStdString(modelName));
    } else {
        debugWindow->logMessage("Failed to load model: " + QString::fromStdString(modelName));
    }
    return modelLoaded;
}

void OllamaAgent::generate(const std::string& modelName, const std::string& prompt, std::function<void(const std::string&)> callback) {
    debugWindow->logMessage("Generating response for model: " + QString::fromStdString(modelName) + " with prompt: " + QString::fromStdString(prompt));
    std::ostringstream responseStream;
    try {
        std::future<void> future = std::async(std::launch::async, [this, modelName, prompt, callback, &responseStream]() {
            debugWindow->logMessage("Starting generation for model: " + QString::fromStdString(modelName) + " with prompt: " + QString::fromStdString(prompt));
            ollamaInstance.generate(modelName, prompt, [this, callback, &responseStream](const std::string& response) {
                debugWindow->logMessage("Received response: " + QString::fromStdString(response));
                responseStream << response;
                if (response.back() == '.') {
                    std::string fullResponse = responseStream.str();
                    QString responseText = QString::fromStdString(fullResponse);
                    QString htmlText = markdownToHtml(responseText);
                    QMetaObject::invokeMethod(QCoreApplication::instance(), [this, callback, htmlText]() {
                        debugWindow->logMessage("Invoking callback with response: " + htmlText);
                        callback(htmlText.toStdString());
                    });
                    responseStream.str("");
                }
            });
            debugWindow->logMessage("Ending generation for model: " + QString::fromStdString(modelName) + " with prompt: " + QString::fromStdString(prompt));
        });
    } catch (const std::exception& e) {
        debugWindow->logMessage("Exception caught during generation: " + QString::fromStdString(e.what()));
        callback("Error: " + std::string(e.what()));
    }
}

QJsonObject OllamaAgent::getSettings() const {
    QJsonObject settings;
    settings["serverURL"] = QString::fromStdString(serverURL);
    settings["use_embeddings"] = useEmbeddingsFlag;
    return settings;
}

void OllamaAgent::setSettings(const QJsonObject& settings) {
    serverURL = settings["serverURL"].toString().toStdString();
    useEmbeddingsFlag = settings["use_embeddings"].toBool();
    ollamaInstance.setServerURL(serverURL);
}

bool OllamaAgent::useEmbeddings() const {
    return useEmbeddingsFlag;
}

void OllamaAgent::setUseEmbeddings(bool useEmbeddings) {
    useEmbeddingsFlag = useEmbeddings;
}

// Define the markdownToHtml function
QString OllamaAgent::markdownToHtml(const QString& markdownText) {
    QTextDocument document;
    document.setMarkdown(markdownText);
    return document.toHtml();
}
