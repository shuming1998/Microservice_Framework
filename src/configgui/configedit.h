#pragma once

#include <QDialog>
#include "ui_configedit.h"

namespace google {
  namespace protobuf {
    class Message;
  }
}

namespace cmsg {
  class CConfig;
}

class ConfigEdit : public QDialog
{
    Q_OBJECT

public:
    ConfigEdit(QWidget *parent = nullptr);
    ~ConfigEdit();

    // ������������������Ļ�ȡ���������ɽ���
    bool loadConfig(const char *ip, int port);

    // ���� message��config ���ɽ���
    void initGui();

signals:
  void addLog(const char *log);
  // ��Ϣ�ص�
  void messageCbSig(bool isOk, const char *msg);

public slots:
  void save();
  // ѡ�� proto �ļ������ض�̬����
  void loadProto();
  // ��Ϣ�ص�
  void messageCb(bool isOk, const char *msg);


private:
    Ui::ConfigEdit ui;
    int configRowCount_ = 0;        // ����������Ϣ������������������� proto �ļ����ɵ�������Ϣ
    google::protobuf::Message *message_ = nullptr;  // ���ڴ洢������
    cmsg::CConfig *config_ = nullptr;               // ���ڴ洢���������Ļ�ȡ��������

};
