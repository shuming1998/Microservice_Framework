#ifndef CDIRSERVICE_HANDLE_H
#define CDIRSERVICE_HANDLE_H
#include "cservicehandle.h"

class CDirServiceHandle : public CServiceHandle {
public:
  CDirServiceHandle();
  ~CDirServiceHandle();

  // �����û���Ŀ¼����Ļص�����
  void dirReqCb(cmsg::CMsgHead *head, CMsg *msg);

  // ����ע��ص�����
  static void regMsgCb();

private:


};

#endif // !CDIRSERVICE_HANDLE_H

