#pragma once

#include <QMainWindow>
#include <QMessageBox>
#include <QGuiApplication>

QT_BEGIN_NAMESPACE
namespace Ui {
class LoginWindow;
}
QT_END_NAMESPACE

class LoginWindow : public QMainWindow
{
    Q_OBJECT

public:
    LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();
    void clearFields();

signals:
    void loginSuccessful();

private slots:
    void on_loginButton_clicked();
    void on_passwordLineEdit_returnPressed();

private:
    Ui::LoginWindow *ui;
    void attemptLogin();
};
