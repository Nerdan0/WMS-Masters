#include "sqlquerywindow.h"
#include "ui_sqlquerywindow.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QElapsedTimer>
#include <QScreen>
#include <QGuiApplication>

SQLQueryWindow::SQLQueryWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SQLQueryWindow)
{
    ui->setupUi(this);

    // Center window on screen
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);

    model = new QSqlQueryModel(this);
    ui->resultsTableView->setModel(model);

    // Set some example queries
    ui->queryTextEdit->setPlainText("-- Example Queries:\n"
                                    "-- SELECT * FROM users;\n"
                                    "-- SELECT * FROM items;\n"
                                    "-- SELECT * FROM orders;\n"
                                    "-- SELECT * FROM order_lines;\n\n");
}

SQLQueryWindow::~SQLQueryWindow()
{
    delete ui;
}

void SQLQueryWindow::on_executeButton_clicked()
{
    QString queryStr = ui->queryTextEdit->toPlainText().trimmed();

    if (queryStr.isEmpty() || queryStr.startsWith("--")) {
        ui->statusLabel->setText("Please enter a valid SQL query.");
        return;
    }

    // Check if the query is a SELECT query
    QString upperQuery = queryStr.toUpper();
    if (!upperQuery.startsWith("SELECT") && !upperQuery.startsWith("WITH")) {
        QMessageBox::warning(this, tr("Query Restriction"),
                             tr("For safety reasons, only SELECT queries are allowed."));
        return;
    }

    // Clear previous results
    model->clear();

    // Execute query
    QSqlQuery query;
    QElapsedTimer timer;
    timer.start();

    bool success = query.exec(queryStr);
    qint64 elapsed = timer.elapsed();

    if (success) {
        // Use std::move to avoid deprecated warning
        model->setQuery(std::move(query));
        int rowCount = model->rowCount();

        // Update status label
        ui->statusLabel->setText(QString("Query executed successfully. Returned %1 %2. Execution time: %3 ms.")
                                     .arg(rowCount)
                                     .arg(rowCount == 1 ? "row" : "rows")
                                     .arg(elapsed));
    } else {
        ui->statusLabel->setText(QString("Error: %1").arg(query.lastError().text()));
    }
}

void SQLQueryWindow::on_clearButton_clicked()
{
    ui->queryTextEdit->clear();
    model->clear();
    ui->statusLabel->clear();
}
