#include "itemswindow.h"
#include "ui_itemswindow.h"
#include <QMessageBox>
#include <QSqlError>
#include <QScreen>
#include <QGuiApplication>
#include <QSqlQuery>

ItemsWindow::ItemsWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ItemsWindow),
    model(nullptr),
    mapper(nullptr),
    isAdding(false)
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

ItemsWindow::~ItemsWindow()
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

void ItemsWindow::setupModel()
{
    model = new QSqlTableModel(this);
    model->setTable("items");

    // Set headers
    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Item Code"));
    model->setHeaderData(2, Qt::Horizontal, tr("Description"));
    model->setHeaderData(3, Qt::Horizontal, tr("Quantity"));
    model->setHeaderData(4, Qt::Horizontal, tr("Price"));

    // Load data
    model->select();

    // Set model to table view
    ui->tableView->setModel(model);

    // Hide ID column
    ui->tableView->hideColumn(0);
}

void ItemsWindow::setupMapper()
{
    mapper = new QDataWidgetMapper(this);
    mapper->setModel(model);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);

    // Map fields to form controls
    mapper->addMapping(ui->idLineEdit, 0);
    mapper->addMapping(ui->codeLineEdit, 1);
    mapper->addMapping(ui->descriptionLineEdit, 2);
    mapper->addMapping(ui->quantitySpinBox, 3);
    mapper->addMapping(ui->priceDoubleSpinBox, 4);
}

void ItemsWindow::enableFormFields(bool enable)
{
    ui->codeLineEdit->setEnabled(enable);
    ui->descriptionLineEdit->setEnabled(enable);
    ui->quantitySpinBox->setEnabled(enable);
    ui->priceDoubleSpinBox->setEnabled(enable);
}

void ItemsWindow::clearForm()
{
    ui->idLineEdit->clear();
    ui->codeLineEdit->clear();
    ui->descriptionLineEdit->clear();
    ui->quantitySpinBox->setValue(0);
    ui->priceDoubleSpinBox->setValue(0.0);
}

void ItemsWindow::updateButtonStates(bool editMode)
{
    ui->addButton->setEnabled(!editMode);
    ui->editButton->setEnabled(!editMode && ui->tableView->currentIndex().isValid());
    ui->deleteButton->setEnabled(!editMode && ui->tableView->currentIndex().isValid());
    ui->saveButton->setEnabled(editMode);
    ui->cancelButton->setEnabled(editMode);
    ui->tableView->setEnabled(!editMode);
}

void ItemsWindow::on_addButton_clicked()
{
    isAdding = true;
    clearForm();
    enableFormFields(true);
    updateButtonStates(true);
    ui->codeLineEdit->setFocus();
}

void ItemsWindow::on_editButton_clicked()
{
    if (!ui->tableView->currentIndex().isValid()) {
        QMessageBox::warning(this, tr("Edit Item"), tr("Please select an item to edit."));
        return;
    }

    isAdding = false;
    enableFormFields(true);
    updateButtonStates(true);
    ui->codeLineEdit->setFocus();
}

void ItemsWindow::on_deleteButton_clicked()
{
    if (!ui->tableView->currentIndex().isValid()) {
        QMessageBox::warning(this, tr("Delete Item"), tr("Please select an item to delete."));
        return;
    }

    int row = ui->tableView->currentIndex().row();
    int itemId = model->data(model->index(row, 0)).toInt();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Delete Item"),
                                  tr("Are you sure you want to delete this item?"),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Check if the item is used in any order lines
        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT COUNT(*) FROM order_lines WHERE item_id = :id");
        checkQuery.bindValue(":id", itemId);

        if (checkQuery.exec() && checkQuery.next()) {
            int count = checkQuery.value(0).toInt();
            if (count > 0) {
                QMessageBox::warning(this, tr("Cannot Delete Item"),
                                     tr("This item cannot be deleted because it is used in %1 order lines. "
                                        "Remove the item from all orders first.").arg(count));
                return;
            }
        }

        model->removeRow(row);
        if (model->submitAll()) {
            model->select();
            clearForm();
        } else {
            QMessageBox::warning(this, tr("Database Error"),
                                 tr("Failed to delete item: %1").arg(model->lastError().text()));
        }
    }
}

void ItemsWindow::on_saveButton_clicked()
{
    // Validate input
    if (ui->codeLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Save Item"), tr("Item Code is required."));
        ui->codeLineEdit->setFocus();
        return;
    }

    if (isAdding) {
        // Add new record
        int row = model->rowCount();
        model->insertRow(row);

        model->setData(model->index(row, 1), ui->codeLineEdit->text());
        model->setData(model->index(row, 2), ui->descriptionLineEdit->text());
        model->setData(model->index(row, 3), ui->quantitySpinBox->value());
        model->setData(model->index(row, 4), ui->priceDoubleSpinBox->value());
    } else {
        // Update existing record
        mapper->submit();
    }

    // Submit changes to database
    if (model->submitAll()) {
        enableFormFields(false);
        updateButtonStates(false);
        isAdding = false;
    } else {
        QMessageBox::warning(this, tr("Database Error"),
                             tr("Failed to save item: %1").arg(model->lastError().text()));
    }
}

void ItemsWindow::on_cancelButton_clicked()
{
    if (isAdding) {
        clearForm();
    } else {
        // Revert to original data
        model->revertAll();
        int currentRow = ui->tableView->currentIndex().row();
        mapper->setCurrentIndex(currentRow);
    }

    enableFormFields(false);
    updateButtonStates(false);
    isAdding = false;
}

void ItemsWindow::on_tableView_clicked(const QModelIndex &index)
{
    if (index.isValid()) {
        mapper->setCurrentIndex(index.row());
        updateButtonStates(false);
    }
}
