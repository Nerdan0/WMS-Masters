#pragma once

#include <QMainWindow>
#include <QVector>
#include <QCloseEvent>
#include "itemswindow.h"
#include "orderswindow.h"
#include "userswindow.h"
#include "sqlquerywindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void show();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_itemsButton_clicked();
    void on_ordersButton_clicked();
    void on_usersButton_clicked();
    void on_sqlQueryButton_clicked();
    void on_logoutButton_clicked();

signals:
    void logoutRequested();

private:
    Ui::MainWindow *ui;
    ItemsWindow* itemsWindow;
    OrdersWindow* ordersWindow;
    UsersWindow* usersWindow;
    SQLQueryWindow* sqlQueryWindow;
    QVector<QWidget*> childWindows;

    void closeAllChildWindows();
};
