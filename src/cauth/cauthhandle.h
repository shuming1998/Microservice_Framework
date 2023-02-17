#ifndef CAUTH_HANDLE_H
#define CAUTH_HANDLE_H
#include "cservicehandle.h"

class CAuthHandle : public CServiceHandle {
public:
  CAuthHandle() {}
  ~CAuthHandle() {}

  // ���յ�¼����
  void loginReq(cmsg::CMsgHead *head, CMsg *msg);

  // ��������û�����
  void addUserReq(cmsg::CMsgHead *head, CMsg *msg);

  static void regMsgCallback() {
    regCb(cmsg::MSG_LOGIN_REQ, (msgCbFunc)&CAuthHandle::loginReq);
    regCb(cmsg::MSG_ADD_USER_REQ, (msgCbFunc)&CAuthHandle::addUserReq);
  }

private:

};





#endif // !CAUTH_HANDLE_H

