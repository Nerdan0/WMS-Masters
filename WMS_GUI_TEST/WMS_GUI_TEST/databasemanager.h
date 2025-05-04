#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager& instance();
    bool initializeDatabase();
    bool validateUser(const QString& username, const QString& password);

    // Users
    bool addUser(const QString& login, const QString& password);
    bool updateUser(int id, const QString& login, const QString& password);
    bool deleteUser(int id);

    // Items
    bool addItem(const QString& code, const QString& description, int quantity, double price);
    bool updateItem(int id, const QString& code, const QString& description, int quantity, double price);
    bool deleteItem(int id);

    // Orders
    bool addOrder(const QString& orderNumber, const QDate& date, const QString& type);
    bool updateOrder(int id, const QString& orderNumber, const QDate& date, const QString& type);
    bool deleteOrder(int id);

    // Order Lines
    bool addOrderLine(int orderId, const QString& orderNumber, int itemId, int quantity);
    bool updateOrderLine(int id, int orderId, const QString& orderNumber, int itemId, int quantity);
    bool deleteOrderLine(int id);

    QSqlQuery executeQuery(const QString& query);

private:
    DatabaseManager(QObject* parent = nullptr);
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool createTables();
    bool populateSampleData();
    QString hashPassword(const QString& password);

    QSqlDatabase m_db;
};
