// ollama_agent.h
#ifndef OLLAMA_AGENT_H
#define OLLAMA_AGENT_H

#include "llm_agent_interface.h"
#include "Ollama.h"
#include <QString>
#include "debugwindow.h" // Include the DebugWindow header

class OllamaAgent : public LlmAgentInterface {
public:
    OllamaAgent(DebugWindow* debugWindow) : debugWindow(debugWindow) {}
    ~OllamaAgent() override = default;

    void setServerURL(const std::string& url) override;
    std::vector<std::string> list_models() override;
    std::vector<std::string> list_running_models() override;
    bool load_model(const std::string& modelName) override;
    void generate(const std::string& modelName, const std::string& prompt, std::function<void(const std::string&)> callback) override;
    QJsonObject getSettings() const override;
    void setSettings(const QJsonObject& settings) override;
    std::string getAgentType() const override { return "Ollama"; } // Implement getAgentType

    // Declare the useEmbeddings and setUseEmbeddings functions
    bool useEmbeddings() const;
    void setUseEmbeddings(bool useEmbeddings);

private:
    Ollama ollamaInstance;
    std::string serverURL;
    bool useEmbeddingsFlag = false; // Initialize the variable
    DebugWindow* debugWindow; // Add DebugWindow pointer

    // Declare the markdownToHtml function
    static QString markdownToHtml(const QString& markdownText);
};

#endif // OLLAMA_AGENT_H
