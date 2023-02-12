#ifndef CSERVICE_PROXY_CLIENT_H
#define CSERVICE_PROXY_CLIENT_H

#include "cserviceclient.h"
#include <map>

class CServiceProxyClient : public CServiceClient {
public:
  virtual void readCb(cmsg::CMsgHead *head, CMsg *msg);

  // �������ݣ���ӱ�ʶ�� callbackTasks_
  virtual bool sendMsg(cmsg::CMsgHead *head, CMsg *msg, CMsgEvent *ev);

  // ע��һ���¼�
  void regEvent(CMsgEvent *ev);

  // ����һ���¼�
  void delEvent(CMsgEvent *ev);

private: 
  // ��Ϣת���Ķ���һ�� proxy ��Ӧ��� CMsgEvent
  // ��ָ���ֵ��Ϊ key��Ҫ���� 64 λ
  std::map<long long, CMsgEvent *> callbackTasks_;    

  std::mutex callbackTasksMtx_;
};


#endif

