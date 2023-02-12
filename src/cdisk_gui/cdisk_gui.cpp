#include "cdisk_gui.h"
#include "cdiskclient.h"
#include <QMessageBox>
#include <QFileDialog>
#include <string>


CDiskGui::CDiskGui(QWidget *parent) : QWidget(parent) {
    ui.setupUi(this);
    CDiskClient::get()->init();

    // ע���ź�֧�ֵ�����
    qRegisterMetaType<std::string>("std::string");

    // �󶨻�ȡĿ¼���ź�
    QObject::connect(CDiskClient::get(), SIGNAL(sDir(std::string)),
                     this, SLOT(updateDir(std::string)));
    // ���ϴ��ļ���ɺ�ˢ�½�����ź�
    QObject::connect(CDiskClient::get(), SIGNAL(sUploadComplete()),
      this, SLOT(refresh()));

    // �������ļ���ɺ�ˢ�½�����ź�
    QObject::connect(CDiskClient::get(), SIGNAL(sDownloadComplete()),
      this, SLOT(downloadComplete()));

    refresh();
}

CDiskGui::~CDiskGui() {}

void CDiskGui::downloadComplete() {
  QMessageBox::information(this, "", "download complete");
}


void CDiskGui::updateDir(std::string dir) {
  //QMessageBox::information(this, "", dir.c_str());

  // ����ȡ���ļ����뵽�ļ��б�,��ʽ���ļ���1,��С1;�ļ���2,��С2;...
  QString str = dir.c_str();
  str = str.trimmed();
  if (str.isEmpty()) {
    return;
  }
  QStringList fileStr = str.split(';');
  ui.filelistWidget->setRowCount(fileStr.size());
  for (int i = 0; i < fileStr.size(); ++i) {
    // �ָ� ',' �������б�
    QStringList fileInfo = fileStr[i].split(',');
    if (fileInfo.size() != 2) {
      continue;
    }
    ui.filelistWidget->setItem(i, 0, new QTableWidgetItem(fileInfo[0]));
    ui.filelistWidget->setItem(i, 1, new QTableWidgetItem(tr("%1Byte").arg(fileInfo[1])));
  }

}

void CDiskGui::updateServerInfo() {
  std::string ip = ui.ipEdit->text().toStdString();
  std::string root = ui.pathEdit->text().toStdString();
  int port = ui.portBox->value();

  CDiskClient::get()->setServerIp(ip);
  CDiskClient::get()->setServerRoot(root);
  CDiskClient::get()->setServerPort(port);
}

void CDiskGui::refresh() {
  updateServerInfo();
  CDiskClient::get()->getDir();

  // 1 ���ӷ�����
  // 2 ���ûص�
}

void CDiskGui::upload() {
  // �û�ѡ��һ���ļ�
  QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("��ѡ���ϴ��ļ�"));
  if (fileName.isEmpty()) {
    return;
  }

  updateServerInfo();
  CDiskClient::get()->uploadFile(fileName.toStdString());

  // ���뵽�ļ��б�
  //ui.filelistWidget->insertRow(0);
  //ui.filelistWidget->setItem(0, 0, new QTableWidgetItem(fileName));
  //ui.filelistWidget->setItem(0, 1, new QTableWidgetItem(tr("%1Byte").arg(1900)));
}

void CDiskGui::download() {
  // �û�ѡ�����ص��ļ���·��
  updateServerInfo();
  int row = ui.filelistWidget->currentRow();
  if (row < 0) {
    QMessageBox::information(this, "", QString::fromLocal8Bit("��ѡ�������ļ�"));
    return;
  }
  // ��ȡѡ����ļ���
  auto item = ui.filelistWidget->item(row, 0);
  std::string fileName = item->text().toStdString();
  // ��ȡ����·��
  QString localPath = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("��ѡ������·��"));
  if (localPath.isEmpty()) {
    return;
  }
  std::string filePath = ui.pathEdit->text().toStdString();
  filePath += "/";
  filePath += fileName;
  CDiskClient::get()->downloadFile(filePath, localPath.toStdString());
}


