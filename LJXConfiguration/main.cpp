#include "mainwindow.h"

#include <QApplication>
#include <QStyleHints>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    a.setStyle("Fusion");
    a.styleHints()->setColorScheme(Qt::ColorScheme::Dark);
#endif

    MainWindow w;
    w.show();
    return QApplication::exec();
}
