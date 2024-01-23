#ifndef LOGIN_H
#define LOGIN_H
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QEventLoop>

#include <QDialog>
#include "config.h"

namespace Ui {
class Login;
}

class Login : public QDialog {
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();

    void initConfig();

    bool checkInput(bool);

signals:
    void try_login();

private slots:
    void checkConnection();
    void checkAuth();

    void on_pushButton_clicked();

    void on_lineEdit_textChanged(const QString &arg1);

    void on_lineEdit_2_textChanged(const QString &arg1);

    void on_lineEdit_3_textChanged(const QString &arg1);

private:
    Ui::Login *ui;
};

#endif // LOGIN_H
