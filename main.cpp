#include "filesystem.h"
#include "login.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Filesystem *w = new Filesystem();
    Login *login = new Login();
    login->show();
    QObject::connect(login, SIGNAL(try_login()), w, SLOT(show()));
    return a.exec();
}
