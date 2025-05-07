#include "orderlineswindow.h"
#include "ui_orderlineswindow.h"
#include "databasemanager.h"
#include <QMessageBox>
#include <QSqlError>
#include <QScreen>
#include <QGuiApplication>
#include <QSqlRelationalDelegate>
#include <QInputDialog>
#include <QCompleter>

OrderLinesWindow::OrderLinesWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrderLinesWindow),
    model(nullptr),
    mapper(nullptr),
    isAdding(false),
    currentOrderId(0),
    currentOrderIndex(0)
{
    ui->setupUi(this);

    // Center window on screen
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);

    // Load all order IDs for navigation
    loadOrderData();

    // Setup item combo box with search
    setupItemComboBox();

    // Initial state
    updateButtonStates(false);
}

OrderLinesWindow::~OrderLinesWindow()
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

void OrderLinesWindow::setupItemComboBox()
{
    QSqlQuery itemsQuery("SELECT id, item_code, item_description FROM items");
    QStringList completionList;

    ui->itemComboBox->clear();
    while (itemsQuery.next()) {
        int id = itemsQuery.value(0).toInt();
        QString code = itemsQuery.value(1).toString();
        QString description = itemsQuery.value(2).toString();
        QString displayText = code + " - " + description;

        ui->itemComboBox->addItem(displayText, id);
        completionList << displayText;
    }

    // Make the combo box editable and add a completer
    ui->itemComboBox->setEditable(true);
    QCompleter *completer = new QCompleter(completionList, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    ui->itemComboBox->setCompleter(completer);

    // Connect item combo box change to update description
    connect(ui->itemComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &OrderLinesWindow::updateItemDescription);
}

void OrderLinesWindow::updateItemDescription(int index)
{
    if (index >= 0) {
        int itemId = ui->itemComboBox->currentData().toInt();
        qDebug()<< "itemId:" <<itemId;
        QSqlQuery query;
        query.prepare("SELECT item_description FROM items WHERE id = :id");
        query.bindValue(":id", itemId);

        if (query.exec() && query.next()) {
            ui->itemDescriptionLineEdit->setText(query.value(0).toString());
            qDebug()<< "Desc:" <<query.value(0).toString();
        } else {
            ui->itemDescriptionLineEdit->clear();
        }
    } else {
        ui->itemDescriptionLineEdit->clear();
    }
}

void OrderLinesWindow::loadOrderData()
{
    orderIdsList.clear();
    orderDataMap.clear();

    QSqlQuery query("SELECT id, order_number, date, type FROM orders ORDER BY id");
    while (query.next()) {
        int orderId = query.value(0).toInt();
        orderIdsList.append(orderId);

        QList<QVariant> orderData;
        orderData.append(query.value(1)); // order_number
        orderData.append(query.value(2)); // date
        orderData.append(query.value(3)); // type

        orderDataMap[orderId] = orderData;
    }
}

void OrderLinesWindow::loadOrder(int orderId)
{
    if (orderId <= 0 || !orderDataMap.contains(orderId)) {
        QMessageBox::warning(this, tr("Load Order"), tr("Invalid order ID."));
        return;
    }

    currentOrderId = orderId;
    currentOrderNumber = orderDataMap[orderId][0].toString();

    // Find the index of this order in the list
    currentOrderIndex = orderIdsList.indexOf(orderId);

    // Update UI
    updateOrderHeaderInfo();
    updateOrderNavigation();

    // Setup model for order lines
    setupLineModel();

    // Select the first row if any exists
    if (model->rowCount() > 0) {
        ui->tableView->selectRow(0);
        on_tableView_clicked(model->index(0, 0));
    } else {
        clearForm();
    }
}

void OrderLinesWindow::loadOrderByNumber(const QString& orderNumber)
{
    if (orderNumber.isEmpty()) {
        QMessageBox::warning(this, tr("Load Order"), tr("Invalid order number."));
        return;
    }

    // Find the order ID for this order number
    QSqlQuery query;
    query.prepare("SELECT id FROM orders WHERE order_number = :order_number");
    query.bindValue(":order_number", orderNumber);

    if (query.exec() && query.next()) {
        int orderId = query.value(0).toInt();
        loadOrder(orderId);
    } else {
        QMessageBox::warning(this, tr("Load Order"), tr("Order number not found."));
    }
}

void OrderLinesWindow::updateOrderHeaderInfo()
{
    if (currentOrderId <= 0 || !orderDataMap.contains(currentOrderId)) {
        return;
    }

    QList<QVariant> orderData = orderDataMap[currentOrderId];
    ui->orderIdEdit->setText(QString::number(currentOrderId));
    ui->orderNumberEdit->setText(orderData[0].toString());
    ui->orderDateEdit->setText(orderData[1].toString());
    ui->orderTypeEdit->setText(orderData[2].toString());

    setWindowTitle(tr("Order Lines - Order #%1").arg(orderData[0].toString()));
}

void OrderLinesWindow::updateOrderNavigation()
{
    ui->prevOrderButton->setEnabled(currentOrderIndex > 0);
    ui->nextOrderButton->setEnabled(currentOrderIndex < orderIdsList.size() - 1);
}

void OrderLinesWindow::setupLineModel()
{
    // If already initialized, just update the model
    if (model) {
        delete model;
        model = nullptr;
    }

    model = new QSqlRelationalTableModel(this);
    model->setTable("order_lines");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // Set relations - change this to show only item code
    model->setRelation(3, QSqlRelation("items", "id", "item_code"));

    // Filter by current order
    model->setFilter(QString("order_id = %1").arg(currentOrderId));

    // Set headers
    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Order ID"));
    model->setHeaderData(2, Qt::Horizontal, tr("Order Number"));
    model->setHeaderData(3, Qt::Horizontal, tr("Item Code"));
    model->setHeaderData(4, Qt::Horizontal, tr("Quantity"));

    // Load data
    model->select();

    // Set model to table view
    ui->tableView->setModel(model);

    // Use custom delegate that makes item code column read-only
    ui->tableView->setItemDelegate(new CustomOrderLinesDelegate(ui->tableView));

    ui->tableView->hideColumn(0); // Hide ID column
    ui->tableView->hideColumn(1); // Hide Order ID column
    ui->tableView->hideColumn(2); // Hide Order Number column

    // Connect to data changes in the model
    connect(model, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex &topLeft, const QModelIndex &bottomRight) {
        // Store the current row before submitting
        int currentRow = ui->tableView->currentIndex().row();

        // When data changes in the view (like editing quantity), submit it to the database
        if (model->submitAll()) {
            // Refresh the data after submitting
            model->select();

            // Reselect the same row after refresh if it still exists
            if (currentRow >= 0 && currentRow < model->rowCount()) {
                ui->tableView->selectRow(currentRow);
                on_tableView_clicked(model->index(currentRow, 0));
            }
        }
    });

    // Setup the mapper
    setupMapper();
}

void OrderLinesWindow::setupMapper()
{
    if (mapper) {
        delete mapper;
        mapper = nullptr;
    }

    mapper = new QDataWidgetMapper(this);
    mapper->setModel(model);

    // Use a regular QSqlRelationalDelegate for the form
    // since we still want to edit the item through the combo box
    mapper->setItemDelegate(new QSqlRelationalDelegate(this));
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);

    // Map fields to form controls
    mapper->addMapping(ui->lineIdEdit, 0); // ID
    mapper->addMapping(ui->itemComboBox, 3); // Item ID
    mapper->addMapping(ui->quantitySpinBox, 4); // Quantity
    // Note: We don't map itemDescriptionLineEdit since it's calculated from itemComboBox
}

void OrderLinesWindow::enableFormFields(bool enable)
{
    ui->itemComboBox->setEnabled(enable);
    // Item description is always read-only as it depends on the item code
    ui->itemDescriptionLineEdit->setEnabled(false);
    ui->quantitySpinBox->setEnabled(enable);
}

void OrderLinesWindow::clearForm()
{
    ui->lineIdEdit->clear();
    ui->itemComboBox->setCurrentIndex(-1);
    ui->itemDescriptionLineEdit->clear();
    ui->quantitySpinBox->setValue(0);
}

void OrderLinesWindow::updateButtonStates(bool editMode)
{
    ui->addLineButton->setEnabled(!editMode);
    ui->editLineButton->setEnabled(!editMode && ui->tableView->currentIndex().isValid());
    ui->deleteLineButton->setEnabled(!editMode && ui->tableView->currentIndex().isValid());
    ui->saveLineButton->setEnabled(editMode);
    ui->cancelLineButton->setEnabled(editMode);
    ui->tableView->setEnabled(!editMode);
    ui->prevOrderButton->setEnabled(!editMode);
    ui->nextOrderButton->setEnabled(!editMode);
    ui->searchOrderButton->setEnabled(!editMode);
    ui->searchOrderEdit->setEnabled(!editMode);
}

void OrderLinesWindow::refreshOrderLinesTable()
{
    model->select();
}

void OrderLinesWindow::on_addLineButton_clicked()
{
    isAdding = true;
    clearForm();
    enableFormFields(true);
    updateButtonStates(true);
    ui->itemComboBox->setFocus();
}

void OrderLinesWindow::on_editLineButton_clicked()
{
    if (!ui->tableView->currentIndex().isValid()) {
        QMessageBox::warning(this, tr("Edit Line"), tr("Please select a line to edit."));
        return;
    }

    isAdding = false;
    enableFormFields(true);
    updateButtonStates(true);
    ui->itemComboBox->setFocus();
}

void OrderLinesWindow::on_deleteLineButton_clicked()
{
    if (!ui->tableView->currentIndex().isValid()) {
        QMessageBox::warning(this, tr("Delete Line"), tr("Please select a line to delete."));
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Delete Line"),
                                  tr("Are you sure you want to delete this order line?"),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        int row = ui->tableView->currentIndex().row();
        model->removeRow(row);
        if (model->submitAll()) {
            refreshOrderLinesTable();
            clearForm();
        } else {
            QMessageBox::warning(this, tr("Database Error"),
                                 tr("Failed to delete order line: %1").arg(model->lastError().text()));
        }
    }
}

void OrderLinesWindow::on_saveLineButton_clicked()
{
    // Validate input
    if (ui->itemComboBox->currentIndex() == -1) {
        QMessageBox::warning(this, tr("Save Line"), tr("Please select an item."));
        ui->itemComboBox->setFocus();
        return;
    }

    if (ui->quantitySpinBox->value() <= 0) {
        QMessageBox::warning(this, tr("Save Line"), tr("Quantity must be greater than zero."));
        ui->quantitySpinBox->setFocus();
        return;
    }

    if (isAdding) {
        // Add new record
        int row = model->rowCount();
        model->insertRow(row);

        model->setData(model->index(row, 1), currentOrderId); // Order ID
        model->setData(model->index(row, 2), currentOrderNumber); // Order Number
        model->setData(model->index(row, 3), ui->itemComboBox->currentData()); // Item ID
        model->setData(model->index(row, 4), ui->quantitySpinBox->value()); // Quantity
    } else {
        // Update existing record (we don't need to update order_id/order_number as they shouldn't change)
        int currentRow = ui->tableView->currentIndex().row();
        model->setData(model->index(currentRow, 3), ui->itemComboBox->currentData()); // Item ID
        model->setData(model->index(currentRow, 4), ui->quantitySpinBox->value()); // Quantity
    }

    // Submit changes to database
    if (model->submitAll()) {
        enableFormFields(false);
        updateButtonStates(false);
        isAdding = false;
        refreshOrderLinesTable();
    } else {
        QMessageBox::warning(this, tr("Database Error"),
                             tr("Failed to save order line: %1").arg(model->lastError().text()));
    }
}

void OrderLinesWindow::on_cancelLineButton_clicked()
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

void OrderLinesWindow::on_tableView_clicked(const QModelIndex &index)
{
    if (index.isValid()) {
        // Get the row
        int row = index.row();

        // Set the mapper's current index
        mapper->setCurrentIndex(row);

        // Get the item ID directly from the model
        QModelIndex itemIdIndex = model->index(row, 3);  // Column 3 contains the item_id
        QVariant itemData = model->data(itemIdIndex, Qt::EditRole);

        // Try to determine if we have an ID or a code
        bool isNumber;
        int itemId = itemData.toInt(&isNumber);

        QSqlQuery query;
        if (isNumber) {
            // We have an ID
            query.prepare("SELECT item_description FROM items WHERE id = :id");
            query.bindValue(":id", itemId);
            qDebug() << "Looking up by item ID:" << itemId;
        } else {
            // We have a code
            QString itemCode = itemData.toString();
            query.prepare("SELECT item_description FROM items WHERE item_code = :item_code");
            query.bindValue(":item_code", itemCode);
            qDebug() << "Looking up by item code:" << itemCode;
        }

        if (query.exec() && query.next()) {
            ui->itemDescriptionLineEdit->setText(query.value(0).toString());
            qDebug() << "Description text:" << query.value(0).toString();
        } else {
            ui->itemDescriptionLineEdit->clear();
            qDebug() << "No description found. Query error:" << query.lastError().text();
        }

        updateButtonStates(false);
    }
}

void OrderLinesWindow::on_prevOrderButton_clicked()
{
    if (currentOrderIndex > 0) {
        currentOrderIndex--;
        loadOrder(orderIdsList[currentOrderIndex]);
    }
}

void OrderLinesWindow::on_nextOrderButton_clicked()
{
    if (currentOrderIndex < orderIdsList.size() - 1) {
        currentOrderIndex++;
        loadOrder(orderIdsList[currentOrderIndex]);
    }
}

void OrderLinesWindow::on_searchOrderButton_clicked()
{
    QString searchText = ui->searchOrderEdit->text().trimmed();
    if (searchText.isEmpty()) {
        QMessageBox::information(this, tr("Search Order"),
                                 tr("Please enter an order number to search."));
        return;
    }

    loadOrderByNumber(searchText);
}
