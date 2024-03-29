#include "filesystem.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QHttpMultiPart>
#include <fmt/core.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include "util.hpp"
#include "./ui_filesystem.h"
#include "uploadselect.h"
#include "addbucket.h"

Filesystem::Filesystem(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::Filesystem) {
    ui->setupUi(this);
    // ui->tableWidget->setColumnCount(2);
    // QStringList horizontalHeaderLabels;
    // horizontalHeaderLabels << "文件名"
    //                        << "目录"
    //                        << "大小"
    //                        << "下载";
    // ui->tableWidget->setHorizontalHeaderLabels(horizontalHeaderLabels);
    ui->tableWidget->setColumnWidth(0, 400);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(ui->tableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(tableDoubleClick(int, int)));
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(rightClickedSlot(QPoint)));
    updateT.setInterval(1000 * 60 * 10);
    connect(&this->updateT, &QTimer::timeout, this, &Filesystem::upgrade);
    updateT.start();
}

Filesystem::~Filesystem() {
    delete ui;
}

void Filesystem::rightClickedSlot(QPoint pos) {
    QTableWidgetItem *item = ui->tableWidget->itemAt(pos);
    if (item == nullptr) {
        if (this->currentBucket.empty())
            this->addBucket();
        return;
    }

    auto row = item->row();
    auto dir = ui->tableWidget->model()->index(row, 1).data().toString();
    auto filename = ui->tableWidget->model()->index(row, 0).data().toString().toStdString();
    if (dir == "否") { // 删除文件
        this->deleteFile(filename);
        return;
    } else if (this->currentKey.empty() && this->currentBucket.empty()) { // 删除bucket
        this->deleteBucket(filename);
    }
}

void Filesystem::init() {
    auto reply = http->get(*config.getBucket());
    connect(reply, &QNetworkReply::finished, this, &Filesystem::parseBucket);
    this->show();
}

void Filesystem::parseBucket() {
    ui->label_2->setText("/");
    QNetworkReply *reply = (QNetworkReply *)(sender());
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::information(this, "tips", "bucket 请求错误");
        return;
    }
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 200) {
        return;
    }
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    auto data = nlohmann::json::parse(reply->readAll().data());
    for (auto it = data.begin(); it != data.end(); it++) {
        auto row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(it->value("name", "").c_str()));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem("是"));
    }
}

void Filesystem::tableDoubleClick(int row, int col) {
    ui->pushButton->setDisabled(false);
    auto file = ui->tableWidget->model()->index(row, 0).data().toString().toStdString();
    auto isDir = ui->tableWidget->model()->index(row, 1).data() == "是" ? true : false;
    if (isDir) { // 目录则进入目录里面
        if (this->currentBucket.empty()) {
            this->currentBucket = fmt::format("/{}", file);
        } else {
            this->currentKey += fmt::format("/{}", file);
        }
        auto key = fmt::format("{}{}", this->currentBucket, this->currentKey);
        QNetworkReply *reply = http->get(*config.getCatalog(key));
        connect(reply, &QNetworkReply::finished, this, &Filesystem::intoDir);
    } else { // 文件则进行下载
        auto key = fmt::format("{}/{}", this->currentKey, file);
        QString dir = QFileDialog::getExistingDirectory(this, tr("保存到"),
                                                        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (dir == "")
            return;
        this->filename = dir + "/" + QString::fromStdString(file);
        QNetworkReply *reply = http->get(*config.getDownload(this->currentBucket, key));
        connect(reply, &QNetworkReply::finished, this, &Filesystem::download);
    }
}

void Filesystem::intoDir() {
    ui->label_2->setText(QString::fromStdString(fmt::format("{}{}", this->currentBucket, this->currentKey)));
    QNetworkReply *reply = (QNetworkReply *)(sender());
    if (reply->error() != QNetworkReply::NoError || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 200) {
        ui->tableWidget->clearContents();
        ui->tableWidget->setRowCount(0);
        return;
    }
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    auto res = nlohmann::json::parse(reply->readAll());
    for (auto it = res.begin(); it != res.end(); it++) {
        auto row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem((*it)["filename"].get<std::string>().c_str()));
        QString isDir = (*it)["is_dir"].get<bool>() ? "是" : "否";
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(isDir));
    }
}

void Filesystem::download() {
    QNetworkReply *reply = (QNetworkReply *)(sender());
    if (reply->error() != QNetworkReply::NoError || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 200) {
        QMessageBox::information(this, "提示", "请求错误");
        return;
    }
    auto data = reply->readAll();
    QFile file(filename);
    defer(file.close());
    file.open(QIODevice::ReadWrite | QIODevice::Text);
    file.write(data);
}

void Filesystem::addBucket() {
    QMenu *pMenu = new QMenu(this);
    pMenu->setContextMenuPolicy(Qt::CustomContextMenu);
    QAction *pAddBucket = new QAction(QString(u8"添加bucket"), this); // 添加bucket
    pMenu->addAction(pAddBucket);
    connect(pAddBucket, &QAction::triggered, [=, this]() {
        AddBucket *addBucket = new AddBucket();
        auto retStatus = addBucket->exec();
        if (retStatus == QDialog::Rejected)
            return;
        auto bucket = addBucket->bucketName.toStdString();
        if (std::count(bucket.begin(), bucket.end(), '/') > 0) {
            QMessageBox::warning(this, "错误", "bucket 名字不能包含特殊字符");
            return;
        }
        if (!bucket.empty() && bucket[0] == '/')
            bucket = bucket.substr(1, bucket.size() - 1);
        nlohmann::json data;
        data["name"] = bucket;
        data["b_type"] = 3;
        QNetworkReply *reply = http->post(*config.addBucket(), QByteArray(data.dump().c_str()));
        connect(reply, &QNetworkReply::finished, [=, this]() {
            reply->deleteLater();
            auto res = reply->error();
            if (reply->error() != QNetworkReply::NoError || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 201) {
                QMessageBox::information(this, "提示", "bucket添加失败");
                return;
            }
            this->flushDir();
        });
    });
    pMenu->exec(QCursor::pos());
    delete pAddBucket;
    delete pMenu;
}

void Filesystem::deleteBucket(std::string bucket) {
    QMenu *pMenu = new QMenu(this);
    pMenu->setContextMenuPolicy(Qt::CustomContextMenu);
    // QAction *pModifyTask = new QAction(QString(u8"修改"), this); // 修改
    QAction *pDelTask = new QAction(QString(u8"删除"), this); // 删除
    pMenu->addAction(pDelTask);
    connect(pDelTask, &QAction::triggered, [=, this]() {
        QNetworkReply *reply = http->deleteResource(*config.deleteBucket(bucket));
        connect(reply, &QNetworkReply::finished, [=, this]() {
            reply->deleteLater();
            if (reply->error() != QNetworkReply::NoError || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 204) {
                QMessageBox::information(this, "提示", "bucket删除失败");
                return;
            }
            this->flushDir();
        });
    });
    pMenu->exec(QCursor::pos());
    delete pDelTask;
    delete pMenu;
}

void Filesystem::deleteFile(std::string filename) {
    QMenu *pMenu = new QMenu(this);
    pMenu->setContextMenuPolicy(Qt::CustomContextMenu);
    // QAction *pModifyTask = new QAction(QString(u8"修改"), this); // 修改
    QAction *pDelTask = new QAction(QString(u8"删除"), this); // 删除
    pMenu->addAction(pDelTask);
    connect(pDelTask, &QAction::triggered, [=, this]() {
        QNetworkReply *reply = http->deleteResource(*config.deleteFile(this->currentBucket, this->currentKey + "/" + filename));
        connect(reply, &QNetworkReply::finished, [=, this]() {
            reply->deleteLater();
            if (reply->error() != QNetworkReply::NoError || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 204) {
                QMessageBox::information(this, "提示", "文件删除失败");
                return;
            }
            this->flushDir();
        });
    });
    pMenu->exec(QCursor::pos());
    delete pDelTask;
    delete pMenu;
}

void Filesystem::flushDir() {
    if (this->currentBucket.empty()) {
        auto reply = http->get(*config.getBucket());
        connect(reply, &QNetworkReply::finished, this, &Filesystem::parseBucket);
        return;
    }
    auto key = fmt::format("{}{}", this->currentBucket, this->currentKey);
    QNetworkReply *reply2 = http->get(*config.getCatalog(key));
    connect(reply2, &QNetworkReply::finished, this, &Filesystem::intoDir);
}

void Filesystem::on_pushButtonBack_clicked() {
    if (this->currentKey.empty()) {
        this->currentBucket = "";
        ui->pushButton->setDisabled(true);
    }
    if (this->currentBucket.empty()) {
        QNetworkReply *reply = http->get(*config.getBucket());
        connect(reply, &QNetworkReply::finished, this, &Filesystem::parseBucket);
        return;
    }
    auto index = std::find_if(this->currentKey.rbegin(), this->currentKey.rend(), [](const char value) {
        return value == '/';
    });
    int len = index - this->currentKey.rbegin() + 1;
    this->currentKey.erase(this->currentKey.end() - len, this->currentKey.end());
    this->flushDir();
}

void Filesystem::on_pushButton_clicked() {
    QString fullname = QFileDialog::getOpenFileName(this,
                                                    tr("选择文件"),
                                                    QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    if (fullname == "")
        return;

    auto uploadSelect = new UploadSelect(this, this->currentBucket, this->currentKey);
    auto status = uploadSelect->exec();
    if (status == QDialog::Rejected) {
        return;
    }

    QFile *file = new QFile(fullname);
    file->open(QIODevice::ReadOnly);
    auto filename = QFileInfo(*file).fileName().toStdString();
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart filePart, bucketPart, keyPart;

    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant(fmt::format("form-data; name=\"file\"; filename=\"{}\"", filename).c_str()));
    filePart.setBodyDevice(file);
    multiPart->append(filePart);

    bucketPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                         QVariant("form-data; name=\"bucket\""));
    auto bucket = uploadSelect->bucket.substr(1, uploadSelect->bucket.size() - 1);
    bucketPart.setBody(bucket.c_str());
    multiPart->append(bucketPart);

    keyPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                      QVariant("form-data; name=\"key\""));
    if (uploadSelect->key.empty())
        keyPart.setBody(filename.c_str());
    else {
        auto key = uploadSelect->key.substr(1, uploadSelect->key.size() - 1) + "/" + filename;
        keyPart.setBody(key.c_str());
    }
    multiPart->append(keyPart);

    QNetworkReply *reply = http->post(*config.postFile(), multiPart);
    connect(reply, &QNetworkReply::finished, [=, this]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 201) {
            QMessageBox::information(this, "提示", "文件上传失败");
        }
        this->flushDir();
    });
}

void Filesystem::upgrade() {
    QNetworkReply *reply = http->get(*config.upgrade());
    connect(reply, &QNetworkReply::finished, [=, this]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 200) {
            return;
        }
        QJsonDocument data = QJsonDocument::fromJson(reply->readAll());
        if (!data.isNull()) {
            auto jsonObj = data.object();
            QString version = jsonObj["version"].toString();
#if WIN32
            QString downloadUrl = jsonObj["download-win"].toString();
#else
            QString downloadUrl = jsonObj["download-linux"].toString();
#endif

            // QString downloadUrl = jsonOb
            if (config.version < version) {
                auto res = QMessageBox::information(this, "upgarde", "check a new version, are you want to upgrade?");
                if (res == QMessageBox::Ok) {
                    // 执行更新程序
                    QProcess fsupgrade;
#if WIN32
                    QString pname = "./fsupgrade.exe";
#else
                    QString pname = "./fsupgrade";
#endif
                    QStringList args;
                    args << downloadUrl;
                    fsupgrade.start(QString("./fsupgrade"), args);
                    // 退出本程序
                    close();
                } else if (res == QMessageBox::Cancel) {
                    this->updateT.stop();
                }
            }
        }
    });
}
