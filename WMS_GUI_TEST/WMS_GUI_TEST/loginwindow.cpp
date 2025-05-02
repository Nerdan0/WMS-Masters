#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "databasemanager.h"
#include <QApplication>
#include <QScreen>
#include <QTimer>

LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LoginWindow)
{
    ui->setupUi(this);

    // Center window on screen
    setFixedSize(width(), height());

    // Use QScreen instead of deprecated QDesktopWidget
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);

    // Connect enter key press on password field to login button
    connect(ui->passwordLineEdit, &QLineEdit::returnPressed, this, &LoginWindow::on_passwordLineEdit_returnPressed);

    // Initialize database
    if (!DatabaseManager::instance().initializeDatabase()) {
        QMessageBox::critical(this, "Database Error", "Failed to initialize database. The application will close.");
        QTimer::singleShot(0, qApp, &QApplication::quit);
    }
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::clearFields()
{
    ui->usernameLineEdit->clear();
    ui->passwordLineEdit->clear();
    ui->statusLabel->clear();
    ui->usernameLineEdit->setFocus();
}

void LoginWindow::on_loginButton_clicked()
{
    attemptLogin();
}

void LoginWindow::on_passwordLineEdit_returnPressed()
{
    attemptLogin();
}

void LoginWindow::attemptLogin()
{
    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        ui->statusLabel->setText("Please enter both username and password");
        return;
    }

    if (DatabaseManager::instance().validateUser(username, password)) {
        ui->statusLabel->clear();
        emit loginSuccessful();
    } else {
        ui->statusLabel->setText("Invalid username or password");
        ui->passwordLineEdit->clear();
        ui->passwordLineEdit->setFocus();
    }
}
