// ollama_api.h
#ifndef OLLAMA_API_H
#define OLLAMA_API_H

#include "Ollama.hpp"
#include "llm_api_interface.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>
#include <QDebug>

namespace ollama {

class OllamaApi : public LlmApiInterface {
public:
    OllamaApi();
    ~OllamaApi();

    void setServerURL(const std::string& url) override;
    std::vector<std::string> list_models() override;
    std::vector<std::string> list_running_models() override;
    bool load_model(const std::string& modelName) override;
    void generate(const std::string& modelName, const std::string& prompt, std::function<void(const std::string&)> callback) override;

    static std::string getServerURL();
    bool pingServer();

private:
    std::string serverURL;
    QNetworkAccessManager* networkManager;

    void generateWithEmbedding(const std::string& modelName, const std::string& prompt, std::function<void(const std::string&)> callback, bool useEmbedding);
};

} // namespace ollama

#endif // OLLAMA_API_H
