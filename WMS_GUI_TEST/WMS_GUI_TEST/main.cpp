#include "loginwindow.h"
#include "mainwindow.h"
#include "databasemanager.h"
#include "itemswindow.h"
#include "orderswindow.h"
#include "userswindow.h"
#include "sqlquerywindow.h"

#include <QApplication>
#include <QScreen>
#include <QGuiApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Set application info
    QApplication::setApplicationName("Warehouse Management System");
    QApplication::setOrganizationName("WMS Corp");

    // Create the main window and login window
    LoginWindow loginWindow;
    MainWindow mainWindow;

    // Connect login success signal to show main window and hide login window
    QObject::connect(&loginWindow, &LoginWindow::loginSuccessful, [&]() {
        loginWindow.hide();
        mainWindow.show();
    });

    // Connect logout request to show login window and hide main window
    QObject::connect(&mainWindow, &MainWindow::logoutRequested, [&]() {
        loginWindow.clearFields();
        loginWindow.show();
    });

    // Show the login window
    loginWindow.show();

    return a.exec();
}
