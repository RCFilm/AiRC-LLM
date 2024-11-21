// llm_api_interface.h
#ifndef LLM_API_INTERFACE_H
#define LLM_API_INTERFACE_H

#include <string>
#include <vector>
#include <functional>

class LlmApiInterface {
public:
    virtual ~LlmApiInterface() = default;
    virtual void setServerURL(const std::string& url) = 0;
    virtual std::vector<std::string> list_models() = 0;
    virtual std::vector<std::string> list_running_models() = 0;
    virtual bool load_model(const std::string& modelName) = 0;
    virtual void generate(const std::string& modelName, const std::string& prompt, std::function<void(const std::string&)> callback) = 0;
};

#endif // LLM_API_INTERFACE_H
