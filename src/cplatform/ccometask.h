#ifndef CCOME_TASK_H
#define CCOME_TASK_H
#include "cmsg.h"
#include "ctask.h"

class CSSLCtx;
class CCOME_API CComeTask : public CTask {
public:
  CComeTask();
  virtual ~CComeTask();

  // ��ʼ���ӷ����������ǵ�Ҫ�Զ����������Խ������߼������ŵ�һ��������
  virtual bool connect();

  virtual bool init();
  virtual void closeBev();

  // ��װ bufferevent_read
  int readMsg(void *data, int size);

  // �¼��ص�����
  virtual void eventCb(short what);

  // ��ȡ���ݻص�
  virtual void readCb() = 0;

  // д�����ݻص�
  virtual void writeCb() {}

  // ���յ���Ϣ������Ϣ�Ļص�����ҵ��������
  // ���� true ���������� false �˳���ǰ����Ϣ������������һ����Ϣ
  /*virtual bool readCb(const CMsg *msg) = 0;*/

  // ������Ϣ
  //virtual bool writeMsg(const CMsg *msg);
  virtual bool writeMsg(const void *data, int size);

  // ���ӳɹ�����Ϣ�ص�
  // ����� ssl ����ͨ�ţ������Ҳ��������������
  virtual void connetedCb() {}

  // �ر���Ϣ����ʱ�����ݽ����͵��˺�������ҵ��ģ������
  virtual void readCb(void *data, int size) {}

  // ����д��ص�
  virtual void beginWrite();

  // �趨��ʱ������ʱ���� timer(������뵽�̳߳�֮��ֻ������һ����ʱ������ init �����е���) 
  // @param ms ��ʱ���õĺ���
  virtual void setTimer(int ms);
  // ��ʱ���Ļص�����
  virtual void timerCb() {}

  void setServerIp(const char *ip);
  void setServerPort(int port) { this->serverPort_ = port; }
  void setIsRecvMsg(bool isRecv) { this->isRecvMsg_ = isRecv; }

  const char *getServerIp() const { return this->serverIp_; };
  const int getServerPort() const { return this->serverPort_; }

  // �ȴ����ӳɹ�
  // @param timeoutSec ���ȴ�ʱ��
  bool waitforConnected(int timeoutSec);

  // �������ӣ�����Ͽ����ٴ�������ֱ�����ӳɹ���ʱ
  bool autoConnect(int timeoutSec);

  bool isConnecting() const { return isConnecting_; }
  bool isConnected() const { return isConnected_; }

  void setAutoDelete(bool is) { this->autoDelete_ = is; }


  // ���� SSL ͨ�������ģ����ʹ���ˣ���ʹ�� SSL ����ͨ��
  void setSslCtx(CSSLCtx *ctx) { this->sslCtx_ = ctx; }
  CSSLCtx *sslCtx() { return this->sslCtx_; }

protected:
  char readBuf_[4096] = { 0 };        // ��������

private:
  CSSLCtx *sslCtx_ = nullptr;         // ssl ͨ�ŵ�������
  bool autoDelete_ = true;            // ���ӶϿ�ʱ�Ƿ��������
  bool initBev(int sock); 
  char serverIp_[16] = { 0 };         // ������ ip ��ַ
  int serverPort_ = 0;                // �������˿ں�
  struct bufferevent *bev_ = nullptr;

  CMsg msg_;                          // ��Ϣ����
  bool isRecvMsg_ = true;             // �Ƿ������Ϣ

  // �ͻ��˵�����״̬ 1δ����(��ʼ����) 2������(�ȴ����ӳɹ�) 3������(������ҵ�����) 4���Ӻ�ʧ��(�ȴ����ʱ�������)
  bool isConnecting_ = true;
  bool isConnected_  = false;
  std::mutex *mtx_ = nullptr;
  struct event *timerEvent_ = nullptr;
};


#endif // !CCOME_TASK_H

