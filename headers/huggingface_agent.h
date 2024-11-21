// huggingface_agent.h
#ifndef HUGGINGFACE_AGENT_H
#define HUGGINGFACE_AGENT_H

#include "llm_agent_interface.h"

class HuggingFaceAgent : public LlmAgentInterface {
public:
    HuggingFaceAgent() = default;
    ~HuggingFaceAgent() override = default;

    void setServerURL(const std::string& url) override;
    std::vector<std::string> list_models() override;
    std::vector<std::string> list_running_models() override;
    bool load_model(const std::string& modelName) override;
    void generate(const std::string& modelName, const std::string& prompt, std::function<void(const std::string&)> callback) override;
    QJsonObject getSettings() const override;
    void setSettings(const QJsonObject& settings) override;
    std::string getAgentType() const override { return "HuggingFace"; } // Implement getAgentType

private:
    std::string serverURL;
    std::string apiKey;
};

#endif // HUGGINGFACE_AGENT_H
