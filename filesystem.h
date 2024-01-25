#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <QMainWindow>
#include "config.h"
#include "bucket.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Filesystem;
}
QT_END_NAMESPACE

class Filesystem : public QMainWindow {
    Q_OBJECT

public:
    Filesystem(QWidget *parent = nullptr);
    ~Filesystem();

public slots:
    void init();
    void parseBucket();
    void tableDoubleClick(int, int);
    void intoDir();
    void download();

private slots:

    void on_pushButtonBack_clicked();

private:
    Ui::Filesystem *ui;
    std::string currentBucket;
    std::string currentKey;
};
#endif // FILESYSTEM_H
