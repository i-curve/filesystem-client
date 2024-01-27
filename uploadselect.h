#ifndef UPLOADSELECT_H
#define UPLOADSELECT_H

#include <QDialog>
#include <string>

namespace Ui {
class UploadSelect;
}

class UploadSelect : public QDialog {
    Q_OBJECT

public:
    explicit UploadSelect(QWidget *parent, std::string, std::string);
    ~UploadSelect();

private:
    Ui::UploadSelect *ui;

public:
    std::string bucket;
    std::string key;
private slots:
    void on_lineEdit_textChanged(const QString &arg1);
    void on_lineEdit_2_textChanged(const QString &arg1);
};

#endif // UPLOADSELECT_H
