#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <QMainWindow>
#include <QFile>
#include <QTimer>
#include "config.h"

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
    void rightClickedSlot(QPoint);
    void init();
    void parseBucket();
    void tableDoubleClick(int, int);
    void intoDir();
    void download();
    void addBucket();
    void deleteBucket(std::string);
    void deleteFile(std::string);
    // flushDir: 刷新当前目录
    void flushDir();

private slots:

    void on_pushButtonBack_clicked();

    void on_pushButton_clicked();
    void upgrade();

private:
    Ui::Filesystem *ui;
    std::string currentBucket;
    std::string currentKey;
    QString filename;
    QTimer updateT;
};
#endif // FILESYSTEM_H
