#ifndef CCOME_TASK_H
#define CCOME_TASK_H
#include "cmsg.h"
#include "ctask.h"
#include <string>

class CCOME_API CComeTask : public CTask {
public:
  bool init() override;
  virtual void closeBev();
  void setServerIp(std::string ip) { this->serverIp_ = ip; }
  void setServerPort(int port) { this->serverPort_ = port; }

  // �¼��ص�����
  virtual void eventCb(short what);

  // ��ȡ���ݻص�
  virtual void readCb();

  // д�����ݻص�
  virtual void writeCb() {}

  // ���յ���Ϣ������Ϣ�Ļص�����ҵ��������
  // ���� true ���������� false �˳���ǰ����Ϣ������������һ����Ϣ
  virtual bool readCb(const CMsg *msg) = 0;

  // ������Ϣ
  virtual bool writeMsg(const CMsg *msg);

  // д���ļ�
  virtual bool writeMsg(const void *data, int size);

  // ���ӳɹ�����Ϣ�ص�
  virtual void connetedCb() {}

  // �ر���Ϣ����ʱ�����ݽ����͵��˺�������ҵ��ģ������
  virtual void readCb(void *data, int size) {}

  // ����д��ص�
  virtual void beginWrite();

  void setIsRecvMsg(bool isRecv) { this->isRecvMsg_ = isRecv; }


protected:
  char readBuf_[4096] = { 0 };         // ��������

private:
  std::string serverIp_ = "";         // ������ ip ��ַ
  int serverPort_ = 0;                // �������˿ں�
  struct bufferevent *bev_ = nullptr;

  CMsg msg_;                          // ��Ϣ����
  bool isRecvMsg_ = true;             // �Ƿ������Ϣ

};


#endif // !CCOME_TASK_H

