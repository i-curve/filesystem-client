#include "addbucket.h"
#include "ui_addbucket.h"

AddBucket::AddBucket(QWidget *parent) : QDialog(parent),
                                        ui(new Ui::AddBucket) {
    ui->setupUi(this);
}

AddBucket::~AddBucket() {
    delete ui;
}

void AddBucket::on_lineEdit_textChanged(const QString &arg1) {
    this->bucketName = arg1;
}
