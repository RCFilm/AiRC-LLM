// huggingface_api.h
#ifndef HUGGINGFACE_API_H
#define HUGGINGFACE_API_H

#include "llm_api_interface.h"

class HuggingFaceApi : public LlmApiInterface {
public:
    HuggingFaceApi() = default;
    ~HuggingFaceApi() override = default;

    void setServerURL(const std::string& url) override;
    std::vector<std::string> list_models() override;
    std::vector<std::string> list_running_models() override;
    bool load_model(const std::string& modelName) override;
    void generate(const std::string& modelName, const std::string& prompt, std::function<void(const std::string&)> callback) override;

private:
    std::string serverURL;
};

#endif // HUGGINGFACE_API_H
