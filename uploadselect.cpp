#include "uploadselect.h"
#include "ui_uploadselect.h"

UploadSelect::UploadSelect(QWidget *parent, std::string bucket, std::string key) : QDialog(parent),
                                                                                   bucket(bucket),
                                                                                   key(key),
                                                                                   ui(new Ui::UploadSelect) {
    ui->setupUi(this);
    ui->lineEdit->setText(QString::fromStdString(bucket));
    ui->lineEdit->setDisabled(true);
    ui->lineEdit_2->setText(QString::fromStdString(key));
}

UploadSelect::~UploadSelect() {
    delete ui;
}

void UploadSelect::on_lineEdit_textChanged(const QString &arg1) {
    this->bucket = arg1.toStdString();
}

void UploadSelect::on_lineEdit_2_textChanged(const QString &arg1) {
    if (!arg1.startsWith("/"))
        this->key = std::string("/") + arg1.toStdString();
    else
        this->key = arg1.toStdString();
}
