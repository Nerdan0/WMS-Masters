#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QDebug>

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Close all child windows
    closeAllChildWindows();

    // Accept the close event
    event->accept();
}

    MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    itemsWindow(nullptr),
    ordersWindow(nullptr),
    usersWindow(nullptr),
    sqlQueryWindow(nullptr)
{
    ui->setupUi(this);

    // Center window on screen
    setFixedSize(width(), height());

    // Use QScreen instead of deprecated QDesktopWidget
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

MainWindow::~MainWindow()
{
    closeAllChildWindows();
    delete ui;
}

void MainWindow::show()
{
    QMainWindow::show();
}

void MainWindow::on_itemsButton_clicked()
{
    if (!itemsWindow) {
        itemsWindow = new ItemsWindow();
        childWindows.append(itemsWindow);
    }

    itemsWindow->show();
    itemsWindow->raise();
    itemsWindow->activateWindow();
}

void MainWindow::on_ordersButton_clicked()
{
    if (!ordersWindow) {
        ordersWindow = new OrdersWindow();
        childWindows.append(ordersWindow);
    }

    ordersWindow->show();
    ordersWindow->raise();
    ordersWindow->activateWindow();
}

void MainWindow::on_usersButton_clicked()
{
    if (!usersWindow) {
        usersWindow = new UsersWindow();
        childWindows.append(usersWindow);
    }

    usersWindow->show();
    usersWindow->raise();
    usersWindow->activateWindow();
}

void MainWindow::on_sqlQueryButton_clicked()
{
    if (!sqlQueryWindow) {
        sqlQueryWindow = new SQLQueryWindow();
        childWindows.append(sqlQueryWindow);
    }

    sqlQueryWindow->show();
    sqlQueryWindow->raise();
    sqlQueryWindow->activateWindow();
}

void MainWindow::on_logoutButton_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Logout", "Are you sure you want to logout?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        closeAllChildWindows();
        hide();
        emit logoutRequested();
    }
}

void MainWindow::closeAllChildWindows()
{
    if (itemsWindow) {
        itemsWindow->close();
        delete itemsWindow;
        itemsWindow = nullptr;
    }

    if (ordersWindow) {
        ordersWindow->close();
        delete ordersWindow;
        ordersWindow = nullptr;
    }

    if (usersWindow) {
        usersWindow->close();
        delete usersWindow;
        usersWindow = nullptr;
    }

    if (sqlQueryWindow) {
        sqlQueryWindow->close();
        delete sqlQueryWindow;
        sqlQueryWindow = nullptr;
    }

    childWindows.clear();
}
