#include "mainwindow.h"

#include <QApplication>
#include <QTimer>
#include <QDebug>
#include <windows.h>
#include <psapi.h>

// Function to retrieve memory usage
SIZE_T GetMemoryUsage() {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize; // WorkingSetSize represents the amount of physical memory used by the process
    }
    return 0;
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Create a timer to periodically log memory usage
    QTimer timer;
    timer.setInterval(5000); // Log every 5 seconds
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        SIZE_T memoryUsage = GetMemoryUsage();
        qDebug() << "Memory Usage:" << memoryUsage << "bytes";
    });
    //timer.start();

    MainWindow w;
    w.showMaximized();
    return a.exec();
}
