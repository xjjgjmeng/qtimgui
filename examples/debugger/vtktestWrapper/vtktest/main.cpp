#include <QApplication>

#include "Widget.h"
#include "TMJTab.h"

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    Widget w;
    w.showMaximized();
    return a.exec();
}
