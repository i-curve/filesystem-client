#ifndef CONFIG_H
#define CONFIG_H
#include <nlohmann/json.hpp>
#include <string>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QNetworkProxy>

inline QNetworkAccessManager *http = new QNetworkAccessManager();

class Config {
public:
    Config();
    Config(std::string user, std::string auth, std::string apiHost, bool remindMe)
        : user(user), auth(auth), apiHost(apiHost), remindMe(remindMe) {}

    void Write(bool);

    QNetworkRequest *getVersion(bool);
    QNetworkRequest *getBucket();

private:
    void WriteNull();
    QNetworkRequest *concat(bool auth, std::string path);

public:
    bool remindMe;
    std::string user;
    std::string auth;
    std::string apiHost;
};

inline Config config;
#endif // CONFIG_H
