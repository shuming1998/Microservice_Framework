#ifndef CCOME_TASK_H
#define CCOME_TASK_H
#include "cmsg.h"
#include "ctask.h"

class CSSLCtx;
class CCOME_API CComeTask : public CTask {
public:
  CComeTask();
  virtual ~CComeTask();

  // ��ʼ���ӷ�����
  // autoConnect �趨���Զ�����
  // @return false: bev ����δ����true: �����������ɹ��������������ӳɹ�
  virtual bool connect();

  // ��ʼ�� bufferevent���ͻ��˽�������
  // ��ӵ��̳߳������б���ã������ͻ��˺ͷ����
  // @return ����˷��� true���ͻ��˵��� Connect ������
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

  // �������ж�ʱ��
  virtual void clearTimers();
  // �趨��ʱ������ʱ���� timer(������뵽�̳߳�֮��ֻ������һ����ʱ������ init �����е���)
  // @param ms ��ʱ���õĺ���
  virtual void setTimer(int ms);
  // ��ʱ���Ļص�����
  virtual void timerCb() {}

  // �����Զ������Ķ�ʱ��
  virtual void setAutoConnectTimer(int ms);
  // �Զ�������ʱ���Ļص�����
  virtual void autoConnectTimerCb();


  void setServerIp(const char *ip);
  void setServerPort(int port) { this->serverPort_ = port; }
  void setIsRecvMsg(bool isRecv) { this->isRecvMsg_ = isRecv; }

  const char *getServerIp() const { return this->serverIp_; };
  const int getServerPort() const { return this->serverPort_; }

  //����IP���ڻ�ȡ������
  // �ͻ��������ӳɹ������� ���Ƿ���˵Ľ������ӵĿͻ���IP��client_ip()��
  void setLocalIp(const char *ip);
  const char *localIp() { return localIp_; };

  // �ȴ����ӳɹ�
  // @param timeoutSec ���ȴ�ʱ��
  bool waitforConnected(int timeoutSec);

  // �������ӣ�����Ͽ����ٴ�������ֱ�����ӳɹ���ʱ
  bool autoConnect(int timeoutSec);

  bool isConnecting() const { return isConnecting_; }
  bool isConnected() const { return isConnected_; }

  // �������ӶϿ�ʱ�Ƿ��Զ�������󣬰�������ʱ���¼�
  void setAutoDelete(bool is) { this->autoDelete_ = is; }
  // �����Զ�������Ĭ�ϲ��Զ�������Ҫ������̳߳�֮ǰ����
  // һ�������Զ�����������Ͳ����Զ�����
  void setAutoConnect(bool is) {
    this->autoConnect_ = is;
    if (is) {
      this->autoDelete_ = false;
    }
  }

  // ���� SSL ͨ�������ģ����ʹ���ˣ���ʹ�� SSL ����ͨ��
  void setSslCtx(CSSLCtx *ctx) { this->sslCtx_ = ctx; }
  CSSLCtx *sslCtx() { return this->sslCtx_; }

  // �趨����ʱ��Ҫ�ڼ����̳߳�֮ǰ
  void setReadTimeoutMs(int ms) { readTimeoutMs_ = ms; }

  //�趨Ҫ�ڼ����̳߳�֮ǰ virtual void timerCb() {}
  void setTimerMs(int ms) { timerMs_ = ms; }

  void setClientIp(const char *ip);
  void setClientPort(int port) { clientPort_ = port; }
  const char *clientIp() const { return clientIp_; }

protected:
  char readBuf_[4096] = { 0 };        // ��������
  char clientIp_[16] = { 0 };         // ������յ����ӣ���ſͻ��˵� IP
  int clientPort_ = 0;                // ������յ����ӣ���ſͻ��˵� Port
  char localIp_[16] = { 0 };          // ���� IP ���ڻ�ȡ������

private:
  bool initBev(int sock);


  int readTimeoutMs_ = 0;             // ����ʱʱ�䣬��ʱ�趨Ҫ�� bufferevent ������֮�󣬼� init ֮��
  CSSLCtx *sslCtx_ = nullptr;         // ssl ͨ�ŵ�������
  bool autoDelete_ = true;            // ���ӶϿ�ʱ�Ƿ��������
  bool autoConnect_ = false;          // �Ƿ��Զ�����
  // �Զ�������ʱ���¼���close ʱ������
  struct event *autoConnectTimerEvent_ = nullptr;

  char serverIp_[16] = { 0 };         // ������ ip ��ַ
  int serverPort_ = 0;                // �������˿ں�
  struct bufferevent *bev_ = nullptr;

  CMsg msg_;                          // ��Ϣ����
  bool isRecvMsg_ = true;             // �Ƿ������Ϣ

  // �ͻ��˵�����״̬ 1δ����(��ʼ����) 2������(�ȴ����ӳɹ�) 3������(������ҵ�����) 4���Ӻ�ʧ��(�ȴ����ʱ�������)
  bool isConnecting_ = true;
  bool isConnected_  = false;
  std::mutex *mtx_ = nullptr;
  // ��ʱ���¼���close ʱ������
  struct event *timerEvent_ = nullptr;
  int timerMs_ = 0;                   // timerCb ��ʱ����ʱ��
};


#endif // !CCOME_TASK_H

