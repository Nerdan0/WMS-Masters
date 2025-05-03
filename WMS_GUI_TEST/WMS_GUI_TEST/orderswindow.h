#pragma once

#include <QWidget>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QDataWidgetMapper>
#include <QDate>
#include "orderlineswindow.h"

namespace Ui {
class OrdersWindow;
}

class OrdersWindow : public QWidget
{
    Q_OBJECT

public:
    explicit OrdersWindow(QWidget *parent = nullptr);
    ~OrdersWindow();

private slots:
    void on_addButton_clicked();
    void on_editButton_clicked();
    void on_deleteButton_clicked();
    void on_saveButton_clicked();
    void on_cancelButton_clicked();
    void on_tableView_clicked(const QModelIndex &index);
    void on_viewLinesButton_clicked();
    void on_tableView_doubleClicked(const QModelIndex &index);

private:
    Ui::OrdersWindow *ui;
    QSqlTableModel *model;
    QDataWidgetMapper *mapper;
    OrderLinesWindow *orderLinesWindow;
    bool isAdding;

    void setupModel();
    void setupMapper();
    void enableFormFields(bool enable);
    void clearForm();
    void updateButtonStates(bool editMode);
    void openOrderLines(int orderId);
};
