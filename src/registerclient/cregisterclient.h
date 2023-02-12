#ifndef CREGISTER_CLIENT_H
#define CREGISTER_CLIENT_H
#include "cserviceclient.h"

// ע�����Ŀͻ����� windows ��ֱ�������ļ�
class CRegisterClient : public CServiceClient {
public:
  // ���ӳɹ�����Ϣ�ص�
  virtual void connetedCb() override;

  // ��ע������ע����񣬴˺�����Ҫ��һ�����ã���������
  // @param serviceName ΢��������
  // @param port ΢����˿�
  // @param ip ΢����ip��������� null,����ÿͻ������ӵ�ַ
  void registerServer(const char *serviceName, int port, const char *ip);

  // ������ȡ΢������Ϣ�б������
  // @param serviceName ΢�������ƣ����Ϊ Null ���� "all"���������ȡ����΢����
  void getServiceReq(const char *serviceName);

  // ��ȡ���еķ����б�����ԭ���ݣ�ÿ�ζ������ϴεĸ�������
  // �˺����Ͳ��� cmsg::CServiceMap ���ݵ��߳���һ���߳���
  // �� pbServiceMap copy ��һ�ݷ����б��û����ʣ������˶��̷߳�������
  cmsg::CServiceMap *getAllService();

  // ��ȡָ���������Ƶ�΢�����б� (����)
  // @param serviceName ��������
  // @param timeoutSec ��ʱʱ��
  // @return �����б�
  // @brief �ٵȴ����ӳɹ� �ڷ��ͻ�ȡ΢�������Ϣ �۵ȴ�΢�����б���Ϣ����(�п����õ���һ�ε�����)
  cmsg::CServiceMap::CServiceList getServices(const char *serviceName, int timeoutSec);

  static CRegisterClient *get() {
    static CRegisterClient *client = nullptr;
    if (!client) {
      client = new CRegisterClient();
    }
    return client;
  }

  // ����ע��������Ӧ��Ϣ
  void registerRes(cmsg::CMsgHead *head, CMsg *msg);

  // ���ջ�ȡ�����б����Ӧ��Ϣ
  void getServiceRes(cmsg::CMsgHead *head, CMsg *msg);

  // ע������Ļص�����
  static void regMsgCallback() {
    regCb(cmsg::MSG_REGISTER_RES, (msgCbFunc)&CRegisterClient::registerRes);
    regCb(cmsg::MSG_GET_SERVICE_RES, (msgCbFunc)&CRegisterClient::getServiceRes);
  }

private:
  CRegisterClient() {}

  char serviceName_[32] = { 0 };
  char serviceIp_[16] = { 0 };
  int servicePort_ = 0;
};

#endif // ! CREGISTER_CLIENT_H

