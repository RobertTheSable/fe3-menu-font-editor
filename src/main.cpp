#include "mainwindow.h"
#include "bgtileset.h"
#include <QApplication>
#include <QFile>
#include <QPixmap>
#include <algorithm>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    if (argc == 4) {
        w.quickOpen(argv[1], argv[2], argv[3]);
    }
    w.show();

    return a.exec();
}
