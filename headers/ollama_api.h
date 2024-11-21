// ollama_api.h
#ifndef OLLAMA_API_H
#define OLLAMA_API_H

#include "Ollama.h"
#include "llm_api_interface.h"

class OllamaApi : public LlmApiInterface {
public:
    OllamaApi() = default;
    ~OllamaApi() override = default;

    void setServerURL(const std::string& url) override;
    std::vector<std::string> list_models() override;
    std::vector<std::string> list_running_models() override;
    bool load_model(const std::string& modelName) override;
    void generate(const std::string& modelName, const std::string& prompt, std::function<void(const std::string&)> callback) override;

private:
    Ollama ollamaInstance;
};

#endif // OLLAMA_API_H
