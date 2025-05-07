#pragma once
#include <QWidget>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QDataWidgetMapper>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QCompleter>
#include <QSqlRelationalDelegate>

namespace Ui {
class OrderLinesWindow;
}

// Custom delegate to make certain columns read-only
class CustomOrderLinesDelegate : public QSqlRelationalDelegate
{
    Q_OBJECT
public:
    CustomOrderLinesDelegate(QObject *parent = nullptr)
        : QSqlRelationalDelegate(parent) {}

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const override
    {
        // Only allow editing for quantity column (column 4)
        if (index.column() != 4) {
            return nullptr; // No editor will be created, making it read-only
        }
        return QSqlRelationalDelegate::createEditor(parent, option, index);
    }
};

class OrderLinesWindow : public QWidget
{
    Q_OBJECT

public:
    explicit OrderLinesWindow(QWidget *parent = nullptr);
    ~OrderLinesWindow();
    void loadOrder(int orderId);
    void loadOrderByNumber(const QString& orderNumber);
    int getCurrentOrderId() const { return currentOrderId; }

private slots:
    void on_addLineButton_clicked();
    void on_editLineButton_clicked();
    void on_deleteLineButton_clicked();
    void on_saveLineButton_clicked();
    void on_cancelLineButton_clicked();
    void on_tableView_clicked(const QModelIndex &index);
    void on_prevOrderButton_clicked();
    void on_nextOrderButton_clicked();
    void on_searchOrderButton_clicked();
    void updateItemDescription(int index);

private:
    Ui::OrderLinesWindow *ui;
    QSqlRelationalTableModel *model;
    QDataWidgetMapper *mapper;
    bool isAdding;
    int currentOrderId;
    QString currentOrderNumber;
    QMap<int, QList<QVariant>> orderDataMap;
    QList<int> orderIdsList;
    int currentOrderIndex;

    void setupOrderModel();
    void setupLineModel();
    void setupMapper();
    void enableFormFields(bool enable);
    void clearForm();
    void updateButtonStates(bool editMode);
    void loadOrderData();
    void updateOrderHeaderInfo();
    void updateOrderNavigation();
    void refreshOrderLinesTable();
    void setupItemComboBox();
};
