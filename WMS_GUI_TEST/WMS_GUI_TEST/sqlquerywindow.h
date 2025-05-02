#pragma once

#include <QWidget>
#include <QSqlQueryModel>

namespace Ui {
class SQLQueryWindow;
}

class SQLQueryWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SQLQueryWindow(QWidget *parent = nullptr);
    ~SQLQueryWindow();

private slots:
    void on_executeButton_clicked();
    void on_clearButton_clicked();

private:
    Ui::SQLQueryWindow *ui;
    QSqlQueryModel *model;
};
