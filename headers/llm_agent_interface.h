// llm_agent_interface.h
#ifndef LLM_AGENT_INTERFACE_H
#define LLM_AGENT_INTERFACE_H

#include <string>
#include <vector>
#include <functional>
#include <QJsonObject>

class LlmAgentInterface {
public:
    virtual ~LlmAgentInterface() = default;
    virtual void setServerURL(const std::string& url) = 0;
    virtual std::vector<std::string> list_models() = 0;
    virtual std::vector<std::string> list_running_models() = 0;
    virtual bool load_model(const std::string& modelName) = 0;
    virtual void generate(const std::string& modelName, const std::string& prompt, std::function<void(const std::string&)> callback) = 0;
    virtual QJsonObject getSettings() const = 0;
    virtual void setSettings(const QJsonObject& settings) = 0;
    virtual std::string getAgentType() const = 0; // New method to get the agent type
};

#endif // LLM_AGENT_INTERFACE_H
