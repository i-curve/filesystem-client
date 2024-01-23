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

    QList<Bucket> GetBuckets();

private:
    Ui::Filesystem *ui;
};
#endif // FILESYSTEM_H
