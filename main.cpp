#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QSharedMemory>
#include "controller.h"
#include "logger.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // create only one instance of the application to avoid controller communication issue
    QSharedMemory sharedMemory("LJXControllerUniqueKey");
    if (!sharedMemory.create(1))
    {
        // An instance is already running
        QMessageBox::warning(nullptr, "Warning", "Another instance of the application is already running.");
        return 0; // Exit if another instance is found
    }

    // create tray icon
    QSystemTrayIcon trayIcon;
    trayIcon.setIcon(QIcon(":/resources/app.png"));
    trayIcon.setToolTip("LJX Controller");

    // add context menu
    QMenu trayMenu;
    QAction *exitAction = trayMenu.addAction("Exit");
    QObject::connect(exitAction, &QAction::triggered, &a, &QApplication::quit);
    trayIcon.setContextMenu(&trayMenu);

    trayIcon.show();
    //
    Controller controller;
    controller.runProcessing();
    QObject::connect(&controller, &Controller::notification, [&trayIcon](const QString &msg) {
        trayIcon.showMessage("Notification", msg);
    });


    sharedMemory.detach();
    return 0;    //a.exec()  // to keep it run in event loop  or 0 to exit on finish processing
}
