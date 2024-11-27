// workspace.cpp
#include "workspace.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

Workspace::Workspace(const QString& name, int id, LlmAgentInterface* agent, const QString& apiType)
    : name(name), model(""), id(id), agent(agent), apiType(apiType) {
    auto space = std::make_unique<hnswlib::L2Space>(embeddingDim);
    index = std::make_unique<hnswlib::HierarchicalNSW<float>>(space.get(), 100000);
    // No need to call initIndex, directly add points if necessary
}

QString Workspace::getName() const {
    return name;
}

void Workspace::setName(const QString& name) {
    this->name = name;
}

QString Workspace::getModel() const {
    return model;
}

void Workspace::setModel(const QString& model) {
    this->model = model;
}

int Workspace::getId() const {
    return id;
}

QString Workspace::getApiType() const {
    return apiType;
}

void Workspace::setApiType(const QString& apiType) {
    this->apiType = apiType;
}

void Workspace::addChatMessage(const QString& message) {
    chatHistory.append(message);
}

QVector<QString> Workspace::getChatHistory() const {
    return chatHistory;
}

QJsonObject Workspace::toJson() const {
    QJsonObject json;
    json["name"] = name;
    json["model"] = model;
    json["id"] = id;
    json["apiType"] = apiType;

    QJsonArray chatArray;
    for (const auto& message : chatHistory) {
        chatArray.append(message);
    }
    json["chatHistory"] = chatArray;

    json["agentSettings"] = agent->getSettings();

    return json;
}

Workspace Workspace::fromJson(const QJsonObject& json, LlmAgentInterface* agent) {
    QString name = json["name"].toString();
    QString model = json["model"].toString();
    int id = json["id"].toInt();
    QString apiType = json["apiType"].toString();
    Workspace workspace(name, id, agent, apiType);
    workspace.setModel(model);

    QJsonArray chatArray = json["chatHistory"].toArray();
    for (const auto& message : chatArray) {
        workspace.addChatMessage(message.toString());
    }

    agent->setSettings(json["agentSettings"].toObject());

    return workspace;
}

LlmAgentInterface* Workspace::getAgent() const {
    return agent;
}

void Workspace::addEmbedding(const std::vector<float>& embedding, const QString& text) {
    qDebug() << "Adding embedding for text:" << text;
    try {
        embeddings.push_back(embedding);
        texts.push_back(text);
        index->addPoint(embedding.data(), texts.size() - 1);
    } catch (const std::exception& e) {
        qCritical() << "Exception caught during embedding addition:" << e.what();
    }
}

std::vector<float> Workspace::getEmbedding(const std::string& text) {
    qDebug() << "Generating embedding for text:" << QString::fromStdString(text);
    try {
        // Implement the logic to generate an embedding for the given text
        // This is a placeholder implementation
        std::vector<float> embedding(embeddingDim, 0.0f);
        return embedding;
    } catch (const std::exception& e) {
        qCritical() << "Exception caught during embedding generation:" << e.what();
        return std::vector<float>();
    }
}

QString Workspace::getNearestText(const std::vector<float>& queryEmbedding) {
    std::priority_queue<std::pair<float, hnswlib::labeltype>> result = index->searchKnn(queryEmbedding.data(), 1);
    if (!result.empty()) {
        hnswlib::labeltype label = result.top().second;
        return texts[label];
    }
    return "";
}

void Workspace::saveIndex(const std::string& filename) {
    index->saveIndex(filename);
}

void Workspace::loadIndex(const std::string& filename) {
    auto space = std::make_unique<hnswlib::L2Space>(embeddingDim);
    index = std::make_unique<hnswlib::HierarchicalNSW<float>>(space.get(), 100000);
    index->loadIndex(filename, space.get());
}

void Workspace::saveToFile(const QString& filename) const {
    QJsonObject json = toJson();
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Could not open file for writing:" << filename;
        return;
    }
    QJsonDocument doc(json);
    file.write(doc.toJson());
    file.close();
}

void Workspace::loadFromFile(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open file for reading:" << filename;
        return;
    }
    QByteArray data = file.readAll();
    file.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qWarning() << "Failed to parse JSON document from file:" << filename;
        return;
    }
    QJsonObject json = doc.object();
    fromJson(json, agent);
}

void Workspace::saveEmbeddings(const QString& filename) const {
    qDebug() << "Saving embeddings to file:" << filename;
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Could not open file for writing:" << filename;
        return;
    }

    QJsonArray embeddingsArray;
    for (const auto& embedding : embeddings) {
        QJsonArray embeddingArray;
        for (const auto& value : embedding) {
            embeddingArray.append(value);
        }
        embeddingsArray.append(embeddingArray);
    }

    QJsonObject json;
    json["embeddings"] = embeddingsArray;
    json["texts"] = QJsonArray::fromStringList(QStringList(texts.begin(), texts.end())); // Convert std::vector<QString> to QStringList

    QJsonDocument doc(json);
    file.write(doc.toJson());
    file.close();
}

void Workspace::loadEmbeddings(const QString& filename) {
    qDebug() << "Loading embeddings from file:" << filename;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open file for reading:" << filename;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qWarning() << "Failed to parse JSON document from file:" << filename;
        return;
    }

    QJsonObject json = doc.object();
    QJsonArray embeddingsArray = json["embeddings"].toArray();
    QJsonArray textsArray = json["texts"].toArray();

    embeddings.clear();
    texts.clear();

    for (const auto& embeddingValue : embeddingsArray) {
        QJsonArray embeddingArray = embeddingValue.toArray();
        std::vector<float> embedding;
        for (const auto& value : embeddingArray) {
            embedding.push_back(value.toDouble());
        }
        embeddings.push_back(embedding);
    }

    for (const auto& textValue : textsArray) {
        texts.push_back(textValue.toString());
    }

    // Rebuild the index
    auto space = std::make_unique<hnswlib::L2Space>(embeddingDim);
    index = std::make_unique<hnswlib::HierarchicalNSW<float>>(space.get(), 100000);
    for (size_t i = 0; i < embeddings.size(); ++i) {
        index->addPoint(embeddings[i].data(), i);
    }
}
