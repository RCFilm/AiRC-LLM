// ollama_api.cpp
#include "ollama_api.h"
#include "Ollama.h"

void OllamaApi::setServerURL(const std::string& url) {
    ollamaInstance.setServerURL(url);
}

std::vector<std::string> OllamaApi::list_models() {
    return ollamaInstance.list_models();
}

std::vector<std::string> OllamaApi::list_running_models() {
    return ollamaInstance.list_running_models();
}

bool OllamaApi::load_model(const std::string& modelName) {
    return ollamaInstance.load_model(modelName);
}

void OllamaApi::generate(const std::string& modelName, const std::string& prompt, std::function<void(const std::string&)> callback) {
    ollamaInstance.generate(modelName, prompt, callback);
}
