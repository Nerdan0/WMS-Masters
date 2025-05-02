#pragma once

#include <QWidget>
#include <QSqlTableModel>
#include <QDataWidgetMapper>

namespace Ui {
class UsersWindow;
}

class UsersWindow : public QWidget
{
    Q_OBJECT

public:
    explicit UsersWindow(QWidget *parent = nullptr);
    ~UsersWindow();

private slots:
    void on_addButton_clicked();
    void on_editButton_clicked();
    void on_deleteButton_clicked();
    void on_saveButton_clicked();
    void on_cancelButton_clicked();
    void on_tableView_clicked(const QModelIndex &index);

private:
    Ui::UsersWindow *ui;
    QSqlTableModel *model;
    QDataWidgetMapper *mapper;
    bool isAdding;
    bool isChangingPassword;

    void setupModel();
    void setupMapper();
    void enableFormFields(bool enable);
    void clearForm();
    void updateButtonStates(bool editMode);
};
