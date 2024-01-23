#include "filesystem.h"
#include "./ui_filesystem.h"

Filesystem::Filesystem(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::Filesystem) {
    ui->setupUi(this);

    ui->tableWidget->setColumnCount(3);
    QStringList horizontalHeaderLabels;
    horizontalHeaderLabels << "文件名"
                           << "大小"
                           << "下载";
    ui->tableWidget->setHorizontalHeaderLabels(horizontalHeaderLabels);
    ui->tableWidget->setColumnWidth(0, 400);
    GetBuckets();
}

Filesystem::~Filesystem() {
    delete ui;
}

QList<Bucket> Filesystem::GetBuckets() {
    QList<Bucket> list;
    QNetworkRequest request(*config.getBucket());
    QNetworkReply *reply = http->get(request);
    reply->deleteLater();
    auto res = reply->readAll();
    qDebug() << QString(res);
    return list;
}
