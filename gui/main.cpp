#include <utility>
#include <type_traits>
#include "mainwindow.h"
#include <QApplication>
// teste

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return QApplication::exec();
}
