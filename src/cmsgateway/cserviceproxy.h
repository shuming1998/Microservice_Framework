#ifndef CSERVICE_PROXY
#define CSERVICE_PROXY

#include "cserviceproxyclient.h"
#include <map>
#include <string>
#include <vector>

class CServiceProxy {
public:
  static CServiceProxy *get() {
    static CServiceProxy cs;
    return &cs;
  }
  // ��ʼ��΢�����б�(��ע�����Ļ�ȡ(Ҳ��һ��΢����))����������
  bool init();

  // ���ؾ����ҵ��ͻ������ӣ��������ݷ���
  bool sendMsg(cmsg::CMsgHead *head, CMsg *msg, CMsgEvent *ev);

  // ������Ϣ�ص�(�ͻ���������ǰ�Ͽ�����ֹ΢��������ѶϿ����ӵĿͻ��˵���Ϣ�ص�)
  void delEvent(CMsgEvent *ev);

  // �����Զ��������߳�
  void start();

  // ֹͣ�߳�
  void stop();

  // 
  void mainFunc();

private:
  bool isExit_ = false;

  std::map<std::string, std::vector<CServiceProxyClient *> > clientMap_;  // ��������΢��������Ӷ���� map
  std::mutex clientMapMtx_;                                               // clientMap_ ������
  std::map<std::string, int> clientMapLastIdx_;                           // ��¼��һ����ѯ������
  std::map<CMsgEvent *, CServiceProxyClient *> clientCbs_;                // �������� callback ����
  std::mutex clientCbsMtx_;                                               // clientCbs_ ������
};


#endif

