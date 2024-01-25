#include <fstream>
#include <QUrlQuery>
#include "config.h"
#include "util.hpp"

Config::Config() {
    try {
        std::ifstream file("config.json");
        defer(file.close());
        auto jsonConfig = nlohmann::json::parse(file);
        user = jsonConfig.value("user", "");
        auth = jsonConfig.value("auth", "");
        apiHost = jsonConfig.value("apiHost", "").c_str();
        this->remindMe = jsonConfig.value("remindMe", false);
    } catch (std::exception e) {
        this->WriteNull();
    }
}

void Config::Write(bool status) {
    if (!status) {
        this->WriteNull();
        return;
    }
    std::ofstream file("config.json");
    defer(file.close());
    nlohmann::json jsonTxt;
    jsonTxt["user"] = this->user;
    jsonTxt["auth"] = this->auth;
    jsonTxt["apiHost"] = this->apiHost;
    jsonTxt["remindMe"] = this->remindMe;
    if (file.is_open()) {
        file << std::setw(4) << jsonTxt;
    }
}

QNetworkRequest *Config::getVersion(bool flag) {
    // return this->concat(flag, "/version");
    auto req = this->concatUrl("/version");
    if (flag)
        this->authRequest(req);
    return req;
}

QNetworkRequest *Config::getBucket() {
    // return this->concat(true, "/bucket");
    auto req = this->concatUrl("/bucket");
    this->authRequest(req);
    return req;
}

QNetworkRequest *Config::getCatalog(std::string path) {
    std::map<std::string, std::string> mp = {{"path", path}};
    auto req = this->concatUrl("/file/catalog", mp);
    this->authRequest(req);
    return req;
}

QNetworkRequest *Config::getDownload(std::string bucket, std::string key) {
    bucket = bucket.substr(1, bucket.size() - 1);
    key = key.substr(1, key.size() - 1);
    std::map<std::string, std::string> mp = {{"bucket", bucket}, {"key", key}};
    auto req = this->concatUrl("/file/download", mp);
    this->authRequest(req);
    return req;
}

void Config::WriteNull() {
    std::ofstream file("config.json");
    defer(file.close());
    if (file.is_open()) {
        file << "{}";
    }
}

QNetworkRequest *Config::authRequest(QNetworkRequest *request) {
    request->setRawHeader("user", this->user.c_str());
    request->setRawHeader("auth", this->auth.c_str());
    return request;
}

QNetworkRequest *Config::concatUrl(const std::string path, std::map<std::string, std::string> query, std::string data) {
    QUrl url = QUrl(apiHost.c_str());
    url.setPath(path.c_str());
    QUrlQuery qquery;
    for (auto [key, value] : query) {
        qquery.addQueryItem(key.c_str(), value.c_str());
    }
    if (!query.empty())
        url.setQuery(qquery);
    QNetworkRequest *req = new QNetworkRequest(url);
    return req;
}
