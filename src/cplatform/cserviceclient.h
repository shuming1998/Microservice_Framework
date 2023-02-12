#ifndef CSERVICE_CLIENT_H
#define CSERVICE_CLIENT_H

#include "cmsgevent.h"
#include "cthreadpool.h"

class CCOME_API CServiceClient : public CMsgEvent {
public:
  CServiceClient();
  ~CServiceClient();
  // ��������뵽�̳߳��У���������
  virtual void startConnect();

private:
  CThreadPool *threadPool_ = nullptr;
};

#endif

