#include "login.h"
#include "ui_login.h"
#include <QMessageBox>
#include <QtConcurrent/QtConcurrent>
#include "util.hpp"

Login::Login(QWidget *parent) : QDialog(parent),
                                ui(new Ui::Login) {
    ui->setupUi(this);
    http->setProxy(QNetworkProxy::NoProxy);
    initConfig();
    ui->lineEdit_2->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    ui->lineEdit->setText(QString::fromStdString(config.user));
    ui->lineEdit_2->setText(QString::fromStdString(config.auth));
    ui->lineEdit_3->setText(QString::fromStdString(config.apiHost));
    QObject::connect(this->ui->pushButton_2, SIGNAL(clicked(bool)), this, SLOT(close()));
    http->get(QNetworkRequest(QUrl("https://baidu.com")))->deleteLater();
}

Login::~Login() {
    delete ui;
}

void Login::initConfig() {
    ui->lineEdit->setText(QString::fromStdString(config.user));
    ui->lineEdit_2->setText(QString::fromStdString(config.auth));
    ui->lineEdit_3->setText(QString::fromStdString(config.apiHost));
    ui->checkBox->setChecked(config.remindMe);
}

bool Login::checkInput(bool flag) {
    auto user = ui->lineEdit->text().toStdString();
    if (user == "") {
        if (flag)
            QMessageBox::information(this, "错误", "user不能为空");
        return false;
    }
    auto auth = ui->lineEdit_2->text().toStdString();
    if (auth == "") {
        if (flag)
            QMessageBox::information(this, "错误", "auth不能为空");
        return false;
    }
    auto apiHost = ui->lineEdit_3->text().toStdString();
    if (apiHost == "") {
        if (flag)
            QMessageBox::information(this, "错误", "apiHost不能为空");
        return false;
    }
    config = Config(user, auth, apiHost, ui->checkBox->isChecked());
    return true;
}

void Login::checkAuth() {
    QNetworkReply *reply = (QNetworkReply *)(sender());
    if (reply->error() != QNetworkReply::NoError || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 200) {
        qDebug() << reply->error();
        qDebug() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        QMessageBox::information(this, "提示", "身份认证失败");
        this->ui->pushButton->setDisabled(false);
        return;
    }
    qDebug() << reply->readAll();
    config.Write(ui->checkBox->isChecked());
    this->close();
    emit try_login();
}

void Login::checkConnection() {
    QNetworkReply *reply = (QNetworkReply *)(sender());
    if (reply->error() != QNetworkReply::NoError || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 200) {
        QMessageBox::information(this, "提示", "apiHost 验证失败");
        defer(this->ui->pushButton->setDisabled(false););
        return;
    }
    auto reply2 = http->get(*config.getVersion(true));
    QObject::connect(reply2, &QNetworkReply::finished, this, &Login::checkAuth);
}

void Login::on_pushButton_clicked() {
    if (auto flag = this->checkInput(true); !flag) {
        return;
    }
    ui->pushButton->setDisabled(true);
    QNetworkReply *reply = http->get(*config.getVersion(false));
    QObject::connect(reply, &QNetworkReply::finished, this, &Login::checkConnection);
}

void Login::on_lineEdit_textChanged(const QString &arg) {
    if (arg.trimmed() != arg)
        ui->lineEdit->setText(arg.trimmed());
}

void Login::on_lineEdit_2_textChanged(const QString &arg) {
    if (arg.trimmed() != arg)
        ui->lineEdit_2->setText(arg.trimmed());
}

void Login::on_lineEdit_3_textChanged(const QString &arg) {
    if (arg.trimmed() != arg)
        ui->lineEdit_3->setText(arg.trimmed());
}
