#ifndef CROUTER_HANDLE_H
#define CROUTER_HANDLE_H

#include "cservicehandle.h"


class CRouterHandle : public CServiceHandle {
public:
  virtual void readCb(cmsg::CMsgHead *head, CMsg *msg);

  // ���ӶϿ�/��ʱ/���� ʱ����
  virtual void close();

private:

};

#endif

