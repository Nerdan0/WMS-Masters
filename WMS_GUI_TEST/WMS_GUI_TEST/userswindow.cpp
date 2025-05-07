#include "userswindow.h"
#include "ui_userswindow.h"
#include "databasemanager.h"
#include <QMessageBox>
#include <QSqlError>
#include <QScreen>
#include <QGuiApplication>

UsersWindow::UsersWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UsersWindow),
    model(nullptr),
    mapper(nullptr),
    isAdding(false),
    isChangingPassword(false)
{
    ui->setupUi(this);

    // Center window on screen
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);

    setupModel();
    setupMapper();

    // Initial state
    updateButtonStates(false);
}

UsersWindow::~UsersWindow()
{
    if (mapper) {
        delete mapper;
        mapper = nullptr;
    }

    if (model) {
        delete model;
        model = nullptr;
    }

    delete ui;
}

void UsersWindow::setupModel()
{
    model = new QSqlTableModel(this);
    model->setTable("users");

    // Set headers
    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Login"));

    // Hide password column from view
    ui->tableView->setColumnHidden(2, true);

    // Load data
    model->select();

    // Set model to table view
    ui->tableView->setModel(model);

    // Hide ID column
    ui->tableView->hideColumn(0);
}

void UsersWindow::setupMapper()
{
    mapper = new QDataWidgetMapper(this);
    mapper->setModel(model);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);

    // Map fields to form controls
    mapper->addMapping(ui->idLineEdit, 0);
    mapper->addMapping(ui->loginLineEdit, 1);
    // We don't map password field as it's hashed in the database

    if (model->rowCount() > 0) {
        ui->tableView->selectRow(0);
        mapper->setCurrentIndex(0);
    }
}

void UsersWindow::enableFormFields(bool enable)
{
    ui->loginLineEdit->setEnabled(enable);
    ui->passwordLineEdit->setEnabled(enable);
    ui->confirmPasswordLineEdit->setEnabled(enable);
}

void UsersWindow::clearForm()
{
    ui->idLineEdit->clear();
    ui->loginLineEdit->clear();
    ui->passwordLineEdit->clear();
    ui->confirmPasswordLineEdit->clear();
}

void UsersWindow::updateButtonStates(bool editMode)
{
    ui->addButton->setEnabled(!editMode);
    ui->editButton->setEnabled(!editMode && ui->tableView->currentIndex().isValid());
    ui->deleteButton->setEnabled(!editMode && ui->tableView->currentIndex().isValid());
    ui->saveButton->setEnabled(editMode);
    ui->cancelButton->setEnabled(editMode);
    ui->tableView->setEnabled(!editMode);
}

void UsersWindow::on_addButton_clicked()
{
    isAdding = true;
    isChangingPassword = true;
    clearForm();
    enableFormFields(true);
    updateButtonStates(true);
    ui->loginLineEdit->setFocus();
}

void UsersWindow::on_editButton_clicked()
{
    if (!ui->tableView->currentIndex().isValid()) {
        QMessageBox::warning(this, tr("Edit User"), tr("Please select a user to edit."));
        return;
    }

    isAdding = false;
    isChangingPassword = false;

    // Ask if user wants to change password
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Change Password"),
                                  tr("Do you want to change the password?"),
                                  QMessageBox::Yes | QMessageBox::No);

    isChangingPassword = (reply == QMessageBox::Yes);

    enableFormFields(true);
    ui->passwordLineEdit->setEnabled(isChangingPassword);
    ui->confirmPasswordLineEdit->setEnabled(isChangingPassword);

    updateButtonStates(true);
    ui->loginLineEdit->setFocus();
}

void UsersWindow::on_deleteButton_clicked()
{
    // Check if this is the last user
    QSqlQuery countQuery("SELECT COUNT(*) FROM users");
    if (countQuery.next() && countQuery.value(0).toInt() <= 1) {
        QMessageBox::warning(this, tr("Delete User"), tr("Cannot delete the last user in the system."));
        return;
    }

    if (!ui->tableView->currentIndex().isValid()) {
        QMessageBox::warning(this, tr("Delete User"), tr("Please select a user to delete."));
        return;
    }

    int row = ui->tableView->currentIndex().row();

    // Check if this is the admin user (ID 1)
    if (model->data(model->index(row, 0)).toInt() == 1) {
        QMessageBox::warning(this, tr("Delete User"), tr("Cannot delete the admin user."));
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Delete User"),
                                  tr("Are you sure you want to delete this user?"),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        model->removeRow(row);
        if (model->submitAll()) {
            model->select();
            clearForm();
        } else {
            QMessageBox::warning(this, tr("Database Error"),
                                 tr("Failed to delete user: %1").arg(model->lastError().text()));
        }
    }
}

void UsersWindow::on_saveButton_clicked()
{
    // Validate input
    if (ui->loginLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Save User"), tr("Login is required."));
        ui->loginLineEdit->setFocus();
        return;
    }

    if (isChangingPassword) {
        if (ui->passwordLineEdit->text().isEmpty()) {
            QMessageBox::warning(this, tr("Save User"), tr("Password is required."));
            ui->passwordLineEdit->setFocus();
            return;
        }

        if (ui->passwordLineEdit->text() != ui->confirmPasswordLineEdit->text()) {
            QMessageBox::warning(this, tr("Save User"), tr("Passwords do not match."));
            ui->confirmPasswordLineEdit->clear();
            ui->passwordLineEdit->clear();
            ui->passwordLineEdit->setFocus();
            return;
        }
    }

    if (isAdding) {
        // Check if login already exists
        QSqlQuery query;
        query.prepare("SELECT COUNT(*) FROM users WHERE login = :login");
        query.bindValue(":login", ui->loginLineEdit->text());

        if (!query.exec() || !query.next()) {
            QMessageBox::warning(this, tr("Database Error"), tr("Failed to check login uniqueness."));
            return;
        }

        if (query.value(0).toInt() > 0) {
            QMessageBox::warning(this, tr("Save User"), tr("Login already exists."));
            ui->loginLineEdit->setFocus();
            return;
        }

        // Add new user
        if (!DatabaseManager::instance().addUser(ui->loginLineEdit->text(), ui->passwordLineEdit->text())) {
            QMessageBox::warning(this, tr("Database Error"), tr("Failed to add user."));
            return;
        }
    } else {
        int id = ui->idLineEdit->text().toInt();
        QString login = ui->loginLineEdit->text();

        if (isChangingPassword) {
            // Update user with new password
            if (!DatabaseManager::instance().updateUser(id, login, ui->passwordLineEdit->text())) {
                QMessageBox::warning(this, tr("Database Error"), tr("Failed to update user."));
                return;
            }
        } else {
            // Update only login
            QSqlQuery query;
            query.prepare("UPDATE users SET login = :login WHERE id = :id");
            query.bindValue(":id", id);
            query.bindValue(":login", login);

            if (!query.exec()) {
                QMessageBox::warning(this, tr("Database Error"),
                                     tr("Failed to update user login: %1").arg(query.lastError().text()));
                return;
            }
        }
    }

    // Refresh model
    model->select();

    // Clear form and update UI state
    clearForm();
    enableFormFields(false);
    updateButtonStates(false);
    isAdding = false;
    isChangingPassword = false;
}

void UsersWindow::on_cancelButton_clicked()
{
    clearForm();
    enableFormFields(false);
    updateButtonStates(false);
    isAdding = false;
    isChangingPassword = false;

    // If editing, reload current record
    if (ui->tableView->currentIndex().isValid()) {
        mapper->setCurrentIndex(ui->tableView->currentIndex().row());
    }
}

void UsersWindow::on_tableView_clicked(const QModelIndex &index)
{
    if (index.isValid()) {
        mapper->setCurrentIndex(index.row());
        updateButtonStates(false);
    }
}
