#ifndef ORDERLINESWINDOW_H
#define ORDERLINESWINDOW_H

#include <QWidget>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QDataWidgetMapper>
#include <QSqlQuery>
#include <QSqlRecord>

namespace Ui {
class OrderLinesWindow;
}

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
};

#endif // ORDERLINESWINDOW_H
