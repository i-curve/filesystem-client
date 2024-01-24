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

    auto res = reply->readAll();
    auto data = nlohmann::json::parse(res.data());
    for (auto it = data.begin(); it != data.end(); it++) {
        auto row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);
        qDebug() << it->value("name", "null").c_str();
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(it->value("name", "").c_str()));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem("是"));
        connect(ui->tableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(tableDoubleClick(int, int)));
    }
}

void Filesystem::tableDoubleClick(int row, int col) {
    auto file = ui->tableWidget->model()->index(row, 0).data().toString().toStdString();
    auto isDir = ui->tableWidget->model()->index(row, 1).data() == "是" ? true : false;
    qDebug() << "is Dir:" << isDir;
    if (isDir) {
        // if (this->currentBucket.empty()) {
        //     this->currentBucket = fmt::format("/{}", file);
        // } else {
        //     this->currentKey += fmt::format("/{}", file);
        // }
        // auto path = fmt::format("{}{}", this->currentBucket, this->currentKey);
        // qDebug() << "path: " << path;
        QNetworkReply *reply = http->get(*config.getCatalog("bucket"));
        connect(reply, &QNetworkReply::finished, this, &Filesystem::intoDir);
    }
}

void Filesystem::intoDir() {
    QNetworkReply *reply = (QNetworkReply *)(sender());
    if (reply->error() != QNetworkReply::NoError || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 200) {
        QMessageBox::information(this, "提示", "请求错误");
        return;
    }
    qDebug() << reply->readAll();
}
