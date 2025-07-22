#include "LoginDialog.h"
#include "ui_LoginDialog.h"
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    ui->pw_line_edit->setEchoMode(QLineEdit::Password);

    connect(ui->login_button, &QPushButton::clicked, this, &LoginDialog::handle_login_button_clicked);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::handle_login_button_clicked()
{
    QString id = ui->id_line_edit->text();
    QString pw = ui->pw_line_edit->text();

    if (id == "admin" && pw == "1234") {
        accept();
    } else {
        QMessageBox::warning(this, "로그인 실패", "ID 또는 비밀번호가 틀렸습니다.");
    }
}
