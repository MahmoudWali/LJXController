#include <QCoreApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QSharedMemory>
#include "controller.h"
#include "logger.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // create only one instance of the application to avoid controller communication issue
    QSharedMemory sharedMemory("LJXControllerUniqueKey");
    if (!sharedMemory.create(1))
    {
        // An instance is already running
        QMessageBox::warning(nullptr, "Warning", "Another instance of the application is already running.");
        return 0; // Exit if another instance is found
    }

    Controller controller;
    QObject::connect(&controller, &Controller::closingApp, [&]() {
        spdlog::shutdown();
        sharedMemory.detach();
        QCoreApplication::exit(0);
        return 0;
    });

    controller.runProcessing();

    if (controller.getCaptureMode() == "single")
    {
        return 0;
    }
    else
    {
        return a.exec() ;    //a.exec()  // to keep it run in event loop  or 0 to exit on finish processing
    }
}
