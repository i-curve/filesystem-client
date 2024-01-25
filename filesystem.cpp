#include "filesystem.h"
#include <QMessageBox>
#include <fmt/core.h>
#include "./ui_filesystem.h"

Filesystem::Filesystem(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::Filesystem) {
    ui->setupUi(this);

    ui->tableWidget->setColumnCount(3);
    QStringList horizontalHeaderLabels;
    horizontalHeaderLabels << "文件名"
                           << "目录"
                           << "大小"
                           << "下载";
    ui->tableWidget->setHorizontalHeaderLabels(horizontalHeaderLabels);
    ui->tableWidget->setColumnWidth(0, 400);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(ui->tableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(tableDoubleClick(int, int)));
}

Filesystem::~Filesystem() {
    delete ui;
}

void Filesystem::init() {
    auto reply = http->get(*config.getBucket());
    connect(reply, &QNetworkReply::finished, this, &Filesystem::parseBucket);
    this->show();
}

void Filesystem::parseBucket() {
    QNetworkReply *reply = (QNetworkReply *)(sender());
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::information(this, "tips", "bucket 请求错误");
        return;
    }
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 200) {
        qDebug() << reply->readAll();
        return;
    }
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    auto res = reply->readAll();
    auto data = nlohmann::json::parse(res.data());
    for (auto it = data.begin(); it != data.end(); it++) {
        auto row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);
        qDebug() << it->value("name", "null").c_str();
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(it->value("name", "").c_str()));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem("是"));
    }
}

void Filesystem::tableDoubleClick(int row, int col) {
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
        QNetworkReply *reply = http->get(*config.getDownload(this->currentBucket, key));
        connect(reply, &QNetworkReply::finished, this, &Filesystem::download);
    }
}

void Filesystem::intoDir() {
    ui->label_2->setText(QString::fromStdString(fmt::format("{}{}", this->currentBucket, this->currentKey)));
    QNetworkReply *reply = (QNetworkReply *)(sender());
    if (reply->error() != QNetworkReply::NoError || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 200) {
        QMessageBox::information(this, "提示", "请求错误");
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
    qDebug() << reply->readAll();
}

void Filesystem::on_pushButtonBack_clicked() {
    if (this->currentKey.empty()) {
        this->currentBucket = "";
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
    auto path = fmt::format("{}{}", this->currentBucket, this->currentKey);
    QNetworkReply *reply = http->get(*config.getCatalog(path));
    connect(reply, &QNetworkReply::finished, this, &Filesystem::intoDir);
}
