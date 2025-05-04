#pragma once

#include <QWidget>
#include <QSqlTableModel>
#include <QDataWidgetMapper>

namespace Ui {
class ItemsWindow;
}

class ItemsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ItemsWindow(QWidget *parent = nullptr);
    ~ItemsWindow();

private slots:
    void on_addButton_clicked();
    void on_editButton_clicked();
    void on_deleteButton_clicked();
    void on_saveButton_clicked();
    void on_cancelButton_clicked();
    void on_tableView_clicked(const QModelIndex &index);

private:
    Ui::ItemsWindow *ui;
    QSqlTableModel *model;
    QDataWidgetMapper *mapper;
    bool isAdding;

    void setupModel();
    void setupMapper();
    void enableFormFields(bool enable);
    void clearForm();
    void updateButtonStates(bool editMode);
};
