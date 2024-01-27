#ifndef ADDBUCKET_H
#define ADDBUCKET_H

#include <QDialog>

namespace Ui {
class AddBucket;
}

class AddBucket : public QDialog {
    Q_OBJECT

public:
    explicit AddBucket(QWidget *parent = nullptr);
    ~AddBucket();

private slots:
    void on_lineEdit_textChanged(const QString &arg1);

private:
    Ui::AddBucket *ui;

public:
    QString bucketName;
};

#endif // ADDBUCKET_H
