// workspace.h
#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <memory>
#include <vector>
#include <queue>
#include <hnswlib/hnswlib.h>
#include "llm_agent_interface.h"

class Workspace {
public:
    Workspace(const QString& name, int id, LlmAgentInterface* agent, const QString& apiType);

    QString getName() const;
    void setName(const QString& name);
    QString getModel() const;
    void setModel(const QString& model);
    int getId() const;
    QString getApiType() const;
    void setApiType(const QString& apiType);
    void addChatMessage(const QString& message);
    QVector<QString> getChatHistory() const;
    QJsonObject toJson() const;
    static Workspace fromJson(const QJsonObject& json, LlmAgentInterface* agent);
    LlmAgentInterface* getAgent() const;
    void addEmbedding(const std::vector<float>& embedding, const QString& text);
    QString getNearestText(const std::vector<float>& queryEmbedding);
    void saveIndex(const std::string& filename);
    void loadIndex(const std::string& filename);
    std::vector<float> getEmbedding(const std::string& text);
    void saveToFile(const QString& filename) const;
    void loadFromFile(const QString& filename);
    void setUseEmbedding(bool useEmbedding);
    void setEnableStreaming(bool enableStreaming);
    void streamAddEmbedding(const std::vector<float>& embedding, const QString& text);

private:
    QString name;
    QString model;
    int id;
    LlmAgentInterface* agent;
    QString apiType;
    QVector<QString> chatHistory;
    std::vector<std::vector<float>> embeddings;
    std::vector<QString> texts;
    std::unique_ptr<hnswlib::HierarchicalNSW<float>> index;
    static constexpr int embeddingDim = 128; // Example dimension, adjust as needed
    bool useEmbedding = true; // Default to true
    bool enableStreaming = true; // Default to true
};

#endif // WORKSPACE_H
