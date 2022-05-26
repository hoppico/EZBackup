#include "ezbackup.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EZBackup w;
    w.show();

    return a.exec();
}
