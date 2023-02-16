#include "clogingui.h"
#include "cauthclient.h"
#include <QMessageBox>
#include <string>

CLoginGui::CLoginGui(QWidget *parent) : QDialog(parent) {
    ui.setupUi(this);
}

CLoginGui::~CLoginGui() {}

void CLoginGui::login() {
  if (ui.usernameEdit->text().isEmpty() || ui.passwordEdit->text().isEmpty()) {
    QMessageBox::information(this, "", QString::fromLocal8Bit("�û��������벻��Ϊ��!"));
    return;
  }

  std::string username = ui.usernameEdit->text().toStdString();
  std::string password = ui.passwordEdit->text().toStdString();
  CAuthClient::get()->LoginReq(username, password);

  cmsg::CLoginRes login;
  bool re = CAuthClient::get()->getLoginInfo(username, &login, 1000);
  if (!re) {
    QMessageBox::information(this, "", QString::fromLocal8Bit("�û��������������!"));
    return;
  }
  std::cout << "Login Success!\n";
  // �ر� dialog ����
  accept();
}


