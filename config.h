#ifndef CONFIG_H
#define CONFIG_H
#define FMT_HEADER_ONLY
#include <nlohmann/json.hpp>
#include <string>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <fmt/core.h>
#include <map>

inline QNetworkAccessManager *http = new QNetworkAccessManager();

class Config {
public:
    Config();
    Config(std::string user, std::string auth, std::string apiHost, bool remindMe)
        : user(user), auth(auth), apiHost(apiHost), remindMe(remindMe) {}

    void Write(bool);

    QNetworkRequest *upgrade();

    QNetworkRequest *getVersion(bool);
    QNetworkRequest *getBucket();
    QNetworkRequest *getCatalog(std::string);
    QNetworkRequest *getDownload(std::string, std::string);
    QNetworkRequest *postFile();
    QNetworkRequest *addBucket();
    QNetworkRequest *deleteBucket(std::string);
    QNetworkRequest *deleteFile(std::string, std::string);

private:
    void WriteNull();
    QNetworkRequest *authRequest(QNetworkRequest *);
    QNetworkRequest *concatUrl(const std::string path, std::map<std::string, std::string> query = std::map<std::string, std::string>(), std::string data = "");

public:
    bool remindMe;
    std::string user;
    std::string auth;
    std::string apiHost;

public:
    QString version = "0.2.0";
};

inline Config config;
#endif // CONFIG_H
