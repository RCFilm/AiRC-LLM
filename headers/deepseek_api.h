// deepseek_api.h
#ifndef DEEPSEEK_API_H
#define DEEPSEEK_API_H

#include "llm_api_interface.h"

class DeepSeekApi : public LlmApiInterface {
public:
    DeepSeekApi() = default;
    ~DeepSeekApi() override = default;

    void setServerURL(const std::string& url) override;
    std::vector<std::string> list_models() override;
    std::vector<std::string> list_running_models() override;
    bool load_model(const std::string& modelName) override;
    void generate(const std::string& modelName, const std::string& prompt, std::function<void(const std::string&)> callback) override;

private:
    std::string serverURL;
};

#endif // DEEPSEEK_API_H
