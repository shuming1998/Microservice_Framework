#ifndef CSERVICE_H
#define CSERVICE_H

#include "ctask.h"
#include "cthreadpool.h"
#include "cservicehandle.h"

class CCOME_API CService : public CTask {
public:
  CService();
  ~CService();
  
  // ÿ�����ӽ���󣬵��ô˺�������������󣬼��뵽�̳߳أ���Ҫ����
  virtual CServiceHandle *createServiceHandle() = 0;

  // �����ʼ�������̳߳ص���
  bool init();

  // ��ʼ�������У������������񣬼��뵽�̳߳�
  bool start();

  void listenCb(int clientSock, struct sockaddr *addr, int socklen);

  // ���÷����������˿�
  void setServerPort(int port) { serverPort_ = port; }

private:
  int threadCount_ = 10;                // �����û����ݵ��̳߳ؿ��ٵ��߳���
  int serverPort_ = 0;                  // �����������˿�

  CThreadPool *threadPoolForListen_;    // ���ڽ����û����ӵ��̳߳أ�����һ���̼߳���
  CThreadPool *threadPoolForClient_;    // ���ڴ����û����ݵ��̳߳�

};

#endif // ! CSERVICE_H

