#ifndef CREGISTER_HANDLE_H
#define CREGISTER_HANDLE_H
#include "cservicehandle.h"

// ����ע�����ĵĿͻ��ˣ���Ӧһ������
class CRegisterHandle : public CServiceHandle {
public:
  // ����
  void heartRes(cmsg::CMsgHead *head, CMsg *msg) {};

  // ���շ����ע������
  void registerReq(cmsg::CMsgHead *head, CMsg *msg);

  // ���շ���ķ�������
  void getServiceReq(cmsg::CMsgHead *head, CMsg *msg);

  static void regMsgCallback() {
    regCb(cmsg::MSG_HEART_REQ, (msgCbFunc)&CRegisterHandle::heartRes);
    regCb(cmsg::MSG_REGISTER_REQ, (msgCbFunc)&CRegisterHandle::registerReq);
    regCb(cmsg::MSG_GET_SERVICE_REQ, (msgCbFunc)&CRegisterHandle::getServiceReq);
  }

};


#endif

