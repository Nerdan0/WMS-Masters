#include "orderswindow.h"
#include "ui_orderswindow.h"
#include <QMessageBox>
#include <QSqlError>
#include <QScreen>
#include <QGuiApplication>
#include <QSqlQuery>

OrdersWindow::OrdersWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrdersWindow),
    orderLinesWindow(nullptr),
    isAdding(false)
{
    ui->setupUi(this);

    // Center window on screen
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);

    // Set current date for dateEdit
    ui->dateEdit->setDate(QDate::currentDate());

    setupModel();
    setupMapper();

    // Initial state
    updateButtonStates(false);

    // Connect double-click signal
    connect(ui->tableView, &QTableView::doubleClicked, this, &OrdersWindow::on_tableView_doubleClicked);
}

OrdersWindow::~OrdersWindow()
{
    if (orderLinesWindow) {
        orderLinesWindow->close();
        delete orderLinesWindow;
    }

    delete mapper;
    delete model;
    delete ui;
}

void OrdersWindow::setupModel()
{
    model = new QSqlTableModel(this);
    model->setTable("orders");

    // Set headers
    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Order Number"));
    model->setHeaderData(2, Qt::Horizontal, tr("Date"));
    model->setHeaderData(3, Qt::Horizontal, tr("Type"));

    // Load data
    model->select();

    // Set model to table view
    ui->tableView->setModel(model);
}

void OrdersWindow::setupMapper()
{
    mapper = new QDataWidgetMapper(this);
    mapper->setModel(model);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);

    // Map fields to form controls
    mapper->addMapping(ui->idLineEdit, 0);
    mapper->addMapping(ui->orderNumberLineEdit, 1);
    // Need a special delegate for date field
    mapper->addMapping(ui->typeComboBox, 3);
}

void OrdersWindow::enableFormFields(bool enable)
{
    ui->orderNumberLineEdit->setEnabled(enable);
    ui->dateEdit->setEnabled(enable);
    ui->typeComboBox->setEnabled(enable);
}

void OrdersWindow::clearForm()
{
    ui->idLineEdit->clear();
    ui->orderNumberLineEdit->clear();
    ui->dateEdit->setDate(QDate::currentDate());
    ui->typeComboBox->setCurrentIndex(0);
}

void OrdersWindow::updateButtonStates(bool editMode)
{
    ui->addButton->setEnabled(!editMode);
    ui->editButton->setEnabled(!editMode && ui->tableView->currentIndex().isValid());
    ui->deleteButton->setEnabled(!editMode && ui->tableView->currentIndex().isValid());
    ui->saveButton->setEnabled(editMode);
    ui->cancelButton->setEnabled(editMode);
    ui->tableView->setEnabled(!editMode);
    ui->viewLinesButton->setEnabled(!editMode && ui->tableView->currentIndex().isValid());
}

void OrdersWindow::on_addButton_clicked()
{
    isAdding = true;
    clearForm();
    enableFormFields(true);
    updateButtonStates(true);
    ui->orderNumberLineEdit->setFocus();
}

void OrdersWindow::on_editButton_clicked()
{
    if (!ui->tableView->currentIndex().isValid()) {
        QMessageBox::warning(this, tr("Edit Order"), tr("Please select an order to edit."));
        return;
    }

    isAdding = false;
    enableFormFields(true);
    updateButtonStates(true);
    ui->orderNumberLineEdit->setFocus();

    // Set date field from model data
    int row = ui->tableView->currentIndex().row();
    QString dateStr = model->data(model->index(row, 2)).toString();
    ui->dateEdit->setDate(QDate::fromString(dateStr, Qt::ISODate));
}

void OrdersWindow::on_deleteButton_clicked()
{
    if (!ui->tableView->currentIndex().isValid()) {
        QMessageBox::warning(this, tr("Delete Order"), tr("Please select an order to delete."));
        return;
    }

    int row = ui->tableView->currentIndex().row();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Delete Order"),
                                  tr("Are you sure you want to delete this order?"),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        model->removeRow(row);
        if (model->submitAll()) {
            model->select();
            clearForm();
        } else {
            QMessageBox::warning(this, tr("Database Error"),
                                 tr("Failed to delete order: %1").arg(model->lastError().text()));
        }
    }
}

void OrdersWindow::on_saveButton_clicked()
{
    // Validate input
    if (ui->orderNumberLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Save Order"), tr("Order Number is required."));
        ui->orderNumberLineEdit->setFocus();
        return;
    }

    if (isAdding) {
        // Add new record
        int row = model->rowCount();
        model->insertRow(row);

        model->setData(model->index(row, 1), ui->orderNumberLineEdit->text());
        model->setData(model->index(row, 2), ui->dateEdit->date().toString(Qt::ISODate));
        model->setData(model->index(row, 3), ui->typeComboBox->currentText());
    } else {
        // Update existing record
        int row = ui->tableView->currentIndex().row();

        model->setData(model->index(row, 1), ui->orderNumberLineEdit->text());
        model->setData(model->index(row, 2), ui->dateEdit->date().toString(Qt::ISODate));
        model->setData(model->index(row, 3), ui->typeComboBox->currentText());
    }

    // Submit changes to database
    if (model->submitAll()) {
        enableFormFields(false);
        updateButtonStates(false);
        isAdding = false;
    } else {
        QMessageBox::warning(this, tr("Database Error"),
                             tr("Failed to save order: %1").arg(model->lastError().text()));
    }
}

void OrdersWindow::on_cancelButton_clicked()
{
    if (isAdding) {
        clearForm();
    } else {
        // Revert to original data
        model->revertAll();
        int currentRow = ui->tableView->currentIndex().row();
        mapper->setCurrentIndex(currentRow);

        // Set date field from model data
        QString dateStr = model->data(model->index(currentRow, 2)).toString();
        ui->dateEdit->setDate(QDate::fromString(dateStr, Qt::ISODate));
    }

    enableFormFields(false);
    updateButtonStates(false);
    isAdding = false;
}

void OrdersWindow::on_tableView_clicked(const QModelIndex &index)
{
    if (index.isValid()) {
        mapper->setCurrentIndex(index.row());

        // Set date field from model data
        QString dateStr = model->data(model->index(index.row(), 2)).toString();
        ui->dateEdit->setDate(QDate::fromString(dateStr, Qt::ISODate));

        updateButtonStates(false);
    }
}

void OrdersWindow::openOrderLines(int orderId)
{
    if (orderId <= 0) {
        QMessageBox::warning(this, tr("View Order Lines"), tr("Invalid order ID."));
        return;
    }

    if (!orderLinesWindow) {
        orderLinesWindow = new OrderLinesWindow();
    }

    // Load the order lines for this order
    orderLinesWindow->loadOrder(orderId);
    orderLinesWindow->show();
    orderLinesWindow->raise();
    orderLinesWindow->activateWindow();
}

void OrdersWindow::on_viewLinesButton_clicked()
{
    if (!ui->tableView->currentIndex().isValid()) {
        QMessageBox::warning(this, tr("View Order Lines"), tr("Please select an order first."));
        return;
    }

    int orderId = model->data(model->index(ui->tableView->currentIndex().row(), 0)).toInt();
    openOrderLines(orderId);
}

void OrdersWindow::on_tableView_doubleClicked(const QModelIndex &index)
{
    if (index.isValid()) {
        int orderId = model->data(model->index(index.row(), 0)).toInt();
        openOrderLines(orderId);
    }
}
