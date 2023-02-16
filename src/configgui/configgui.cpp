#include "configgui.h"
#include "ctools.h"
#include "clogclient.h"
#include "cconfigclient.h"
#include "configedit.h"
#include "clogingui.h"
#include <QMouseEvent>
#include <QMessageBox>
#include <QTime>
#include <string>
#include <sstream>

ConfigGui::ConfigGui(QWidget *parent) : QWidget(parent) {
    ui.setupUi(this);

    // ȥ��ԭ���ڱ߿�
    setWindowFlags(Qt::FramelessWindowHint);

    // ���ر�����������ʾԲ��
    setAttribute(Qt::WA_TranslucentBackground);

    // ���ô��ڿ��϶�
    // ��������¼�
    setMouseTracking(true);

    refresh();
}

ConfigGui::~ConfigGui() {}

static bool mousePress = false;
static QPoint mousePoint;

void ConfigGui::mouseMoveEvent(QMouseEvent *ev) {
  // û�а��£�����ԭ�¼�
  if (!mousePress) {
    QWidget::mouseMoveEvent(ev);
    return;
  }
  auto curPos = ev->globalPos();
  this->move(curPos - mousePoint);
}

void ConfigGui::mousePressEvent(QMouseEvent *ev) {
  // ���������£���¼λ��
  if (ev->button() == Qt::LeftButton) {
    mousePress = true;
    mousePoint = ev->pos();
  }
}

void ConfigGui::mouseReleaseEvent(QMouseEvent *ev) {
  mousePress = false;
}

void ConfigGui::refresh() {
  // 1 ������ʷ�б�
  addLog("������ʷ�б�");
  while (ui.tableWidget->rowCount() > 0) {
    ui.tableWidget->removeRow(0);
  }
  
  // 2 ����޸����������ĵ� IP ��˿ڣ��Ͽ�����
  std::string serverIp = ui.serverIpEdit->text().toStdString();
  int serverPort = ui.servicePortBox->value();
  std::stringstream ss;
  ss << serverIp << ':' << serverPort;
  LOG_DEBUG(ss.str().c_str());

  // �򿪼�Ȩ���ڣ���½��֤
  CLoginGui gui;
  if (gui.exec() != QDialog::Accepted) {
    return;
  }


  // �Ͽ�֮ǰ�����ӣ����½�������
  CConfigClient::get()->setServerIp(serverIp.c_str());
  CConfigClient::get()->setServerPort(serverPort);
  // �Ͽ�����ʱ��Ҫ�������
  CConfigClient::get()->setAutoDelete(false);
  CConfigClient::get()->close();
  if (!CConfigClient::get()->autoConnect(3)) {
    addLog("������������ʧ��");
    return;
  }
  addLog("�����������ĳɹ�");

  // 3 ���������Ļ�ȡ�����б�
  auto configs = CConfigClient::get()->downloadAllConfig(1, 10000, 10);

  // 4 �����ȡ���б�
  int rows = configs.config_size();
  ui.tableWidget->setRowCount(rows);
  for (int i = 0; i < rows; ++i) {
    auto config = configs.config(i);
    ui.tableWidget->setItem(i, 0, new QTableWidgetItem(config.servicename().c_str()));
    ui.tableWidget->setItem(i, 1, new QTableWidgetItem(config.serviceip().c_str()));
    std::string servicePort = std::to_string(config.serviceport());
    ui.tableWidget->setItem(i, 2, new QTableWidgetItem(servicePort.c_str()));
  }
  addLog("���������б����");
}

void ConfigGui::addLog(const char *log) {
  // ����������ʾ
  QString str = QTime::currentTime().toString("HH:mm:ss");
  str += " ";
  str += QString::fromLocal8Bit(log);
  LOG_DEBUG(log);
  ui.logListWidget->insertItem(0, new QListWidgetItem(str));
}

void ConfigGui::addConfig() {
  // ��ģ̬���ڣ��ȴ��˳�
  ConfigEdit edit;
  if (edit.exec() == QDialog::Accepted) {
    addLog("�������óɹ�");
  }

  refresh();
} 

void ConfigGui::delConfig() {
  if (ui.tableWidget->rowCount() == 0) {
    return;
  }
  int row = ui.tableWidget->currentRow();
  if (row < 0) {
    return;
  }

  // ��ȡѡ�е����ã�name IP port
  auto itemName = ui.tableWidget->item(row, 0);
  auto itemIp = ui.tableWidget->item(row, 1);
  auto itemPort = ui.tableWidget->item(row, 2);
  std::string name = itemName->text().toStdString();
  std::string ip = itemIp->text().toStdString();
  int port = atoi(itemPort->text().toStdString().c_str());

  std::stringstream ss;
  ss << "��ȷ��ɾ��΢�������ã�[" << name << '|' << ip << ':' << port << "] !";
  if (QMessageBox::information(0, "", QString::fromLocal8Bit(ss.str().c_str()),
                               QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
    return; 
  }
  CConfigClient::get()->deleteConfig(ip.c_str(), port);

  std::stringstream sss;
  sss << "ɾ��΢��������: " << name << '|' << ip << ':' << port << "]";
  addLog(sss.str().c_str());
  refresh();
}

void ConfigGui::editConfig() {
  if (ui.tableWidget->rowCount() == 0) {
    return;
  }
  // ��ȡ��Ҫ�༭������
  int row = ui.tableWidget->currentRow();
  if (row < 0) {
    return;
  }

  // ��ȡѡ�е����ã�IP port
  auto itemIp = ui.tableWidget->item(row, 1);
  auto itemPort = ui.tableWidget->item(row, 2);
  std::string ip = itemIp->text().toStdString();
  int port = atoi(itemPort->text().toStdString().c_str());

  //LOG_DEBUG(ip.c_str());

  // �����ý���
  ConfigEdit edit;
  if (!edit.loadConfig(ip.c_str(), port)) {
    addLog("��ȡ����ʧ��!");
    return;
  }
  if (edit.exec() == QDialog::Accepted) {
    addLog("�޸����óɹ�");
  }

  refresh();
}