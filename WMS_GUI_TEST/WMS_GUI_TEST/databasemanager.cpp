#include "databasemanager.h"

#include <QStandardPaths>
#include <QDir>
#include <QDateTime>

DatabaseManager::DatabaseManager(QObject* parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dbPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    m_db.setDatabaseName(dbPath + "/wms.db");
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::initializeDatabase()
{
    if (!m_db.open()) {
        qDebug() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery query;
    query.exec("PRAGMA foreign_keys = ON");

    QFile dbFile(m_db.databaseName());
    bool dbExists = dbFile.exists() && dbFile.size() > 0;

    if (!dbExists) {
        qDebug() << "Creating new database...";
        if (!createTables()) {
            qDebug() << "Failed to create tables";
            return false;
        }

        if (!populateSampleData()) {
            qDebug() << "Failed to populate sample data";
            return false;
        }

        qDebug() << "Database initialized successfully";
    } else {
        qDebug() << "Database already exists";
    }

    return true;
}

QString DatabaseManager::hashPassword(const QString& password)
{
    QByteArray passwordBytes = password.toUtf8();
    QByteArray hash = QCryptographicHash::hash(passwordBytes, QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

bool DatabaseManager::createTables()
{
    QSqlQuery query;

    // Users table
    if (!query.exec("CREATE TABLE IF NOT EXISTS users ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "login TEXT UNIQUE NOT NULL, "
                    "password TEXT NOT NULL)")) {
        qDebug() << "Failed to create users table:" << query.lastError().text();
        return false;
    }

    // Items table
    if (!query.exec("CREATE TABLE IF NOT EXISTS items ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "item_code TEXT UNIQUE NOT NULL, "
                    "item_description TEXT, "
                    "quantity INTEGER DEFAULT 0, "
                    "price REAL DEFAULT 0)")) {
        qDebug() << "Failed to create items table:" << query.lastError().text();
        return false;
    }

    // Orders table
    if (!query.exec("CREATE TABLE IF NOT EXISTS orders ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "order_number TEXT UNIQUE NOT NULL, "
                    "date TEXT NOT NULL, "
                    "type TEXT NOT NULL)")) {
        qDebug() << "Failed to create orders table:" << query.lastError().text();
        return false;
    }

    // Order Lines table
    if (!query.exec("CREATE TABLE IF NOT EXISTS order_lines ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "order_id INTEGER NOT NULL, "
                    "order_number TEXT NOT NULL, "
                    "item_id INTEGER NOT NULL, "
                    "quantity INTEGER NOT NULL, "
                    "FOREIGN KEY (order_id) REFERENCES orders(id), "
                    "FOREIGN KEY (order_number) REFERENCES orders(order_number), "
                    "FOREIGN KEY (item_id) REFERENCES items(id))")) {
        qDebug() << "Failed to create order_lines table:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::populateSampleData()
{
    // Add admin user
    if (!addUser("admin", "admin123")) {
        qDebug() << "Failed to add admin user";
        return false;
    }

    // Add sample users
    if (!addUser("user1", "password1")) {
        qDebug() << "Failed to add sample user";
        return false;
    }

    // Add sample items
    if (!addItem("IT001", "Laptop", 10, 1200.00)) {
        qDebug() << "Failed to add sample item 1";
        return false;
    }

    if (!addItem("IT002", "Mouse", 50, 25.00)) {
        qDebug() << "Failed to add sample item 2";
        return false;
    }

    if (!addItem("IT003", "Keyboard", 30, 45.00)) {
        qDebug() << "Failed to add sample item 3";
        return false;
    }

    // Add sample orders
    QDate today = QDate::currentDate();
    if (!addOrder("ORD001", today, "to")) {
        qDebug() << "Failed to add sample order 1";
        return false;
    }

    if (!addOrder("ORD002", today.addDays(-1), "from")) {
        qDebug() << "Failed to add sample order 2";
        return false;
    }

    // Add sample order lines with updated function signature
    if (!addOrderLine(1, "ORD001", 1, 2)) { // 2 laptops for order ORD001
        qDebug() << "Failed to add order line 1";
        return false;
    }

    if (!addOrderLine(1, "ORD001", 2, 5)) { // 5 mice for order ORD001
        qDebug() << "Failed to add order line 2";
        return false;
    }

    if (!addOrderLine(2, "ORD002", 3, 3)) { // 3 keyboards for order ORD002
        qDebug() << "Failed to add order line 3";
        return false;
    }

    return true;
}

bool DatabaseManager::validateUser(const QString& username, const QString& password)
{
    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE login = :login");
    query.bindValue(":login", username);

    if (!query.exec() || !query.next()) {
        return false;
    }

    QString storedHash = query.value(0).toString();
    QString inputHash = hashPassword(password);

    return storedHash == inputHash;
}

bool DatabaseManager::addUser(const QString& login, const QString& password)
{
    QSqlQuery query;
    query.prepare("INSERT INTO users (login, password) VALUES (:login, :password)");
    query.bindValue(":login", login);
    query.bindValue(":password", hashPassword(password));

    if (!query.exec()) {
        qDebug() << "Failed to add user:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::updateUser(int id, const QString& login, const QString& password)
{
    QSqlQuery query;
    query.prepare("UPDATE users SET login = :login, password = :password WHERE id = :id");
    query.bindValue(":id", id);
    query.bindValue(":login", login);
    query.bindValue(":password", hashPassword(password));

    if (!query.exec()) {
        qDebug() << "Failed to update user:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::deleteUser(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM users WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Failed to delete user:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::addItem(const QString& code, const QString& description, int quantity, double price)
{
    QSqlQuery query;
    query.prepare("INSERT INTO items (item_code, item_description, quantity, price) "
                  "VALUES (:code, :description, :quantity, :price)");
    query.bindValue(":code", code);
    query.bindValue(":description", description);
    query.bindValue(":quantity", quantity);
    query.bindValue(":price", price);

    if (!query.exec()) {
        qDebug() << "Failed to add item:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::updateItem(int id, const QString& code, const QString& description, int quantity, double price)
{
    QSqlQuery query;
    query.prepare("UPDATE items SET item_code = :code, item_description = :description, "
                  "quantity = :quantity, price = :price WHERE id = :id");
    query.bindValue(":id", id);
    query.bindValue(":code", code);
    query.bindValue(":description", description);
    query.bindValue(":quantity", quantity);
    query.bindValue(":price", price);

    if (!query.exec()) {
        qDebug() << "Failed to update item:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::deleteItem(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM items WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Failed to delete item:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::addOrder(const QString& orderNumber, const QDate& date, const QString& type)
{
    QSqlQuery query;
    query.prepare("INSERT INTO orders (order_number, date, type) VALUES (:order_number, :date, :type)");
    query.bindValue(":order_number", orderNumber);
    query.bindValue(":date", date.toString(Qt::ISODate));
    query.bindValue(":type", type);

    if (!query.exec()) {
        qDebug() << "Failed to add order:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::updateOrder(int id, const QString& orderNumber, const QDate& date, const QString& type)
{
    QSqlQuery query;
    query.prepare("UPDATE orders SET order_number = :order_number, date = :date, type = :type WHERE id = :id");
    query.bindValue(":id", id);
    query.bindValue(":order_number", orderNumber);
    query.bindValue(":date", date.toString(Qt::ISODate));
    query.bindValue(":type", type);

    if (!query.exec()) {
        qDebug() << "Failed to update order:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::deleteOrder(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM orders WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Failed to delete order:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::addOrderLine(int orderId, const QString& orderNumber, int itemId, int quantity)
{
    QSqlQuery query;
    query.prepare("INSERT INTO order_lines (order_id, order_number, item_id, quantity) VALUES (:order_id, :order_number, :item_id, :quantity)");
    query.bindValue(":order_id", orderId);
    query.bindValue(":order_number", orderNumber);
    query.bindValue(":item_id", itemId);
    query.bindValue(":quantity", quantity);

    if (!query.exec()) {
        qDebug() << "Failed to add order line:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::updateOrderLine(int id, int orderId, const QString& orderNumber, int itemId, int quantity)
{
    QSqlQuery query;
    query.prepare("UPDATE order_lines SET order_id = :order_id, order_number = :order_number, item_id = :item_id, quantity = :quantity WHERE id = :id");
    query.bindValue(":id", id);
    query.bindValue(":order_id", orderId);
    query.bindValue(":order_number", orderNumber);
    query.bindValue(":item_id", itemId);
    query.bindValue(":quantity", quantity);

    if (!query.exec()) {
        qDebug() << "Failed to update order line:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::deleteOrderLine(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM order_lines WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Failed to delete order line:" << query.lastError().text();
        return false;
    }

    return true;
}

QSqlQuery DatabaseManager::executeQuery(const QString& queryStr)
{
    QSqlQuery query;
    query.exec(queryStr);
    return query;
}
