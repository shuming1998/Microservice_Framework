#ifndef CCONFIG_HANDLE_H
#define CCONFIG_HANDLE_H

#include "cservicehandle.h"


class CConfigHandle : public CServiceHandle {
public:
  // �����ϴ����õ���Ϣ
  void uploadConfig(cmsg::CMsgHead *head, CMsg *msg);

  // �����������õ���Ϣ
  void downloadConfig(cmsg::CMsgHead *head, CMsg *msg);

  // ���������������õ���Ϣ(��ҳ)
  void downloadAllConfig(cmsg::CMsgHead *head, CMsg *msg);

  // ����ɾ��ָ��΢�������õ���Ϣ
  void deleteConfig(cmsg::CMsgHead *head, CMsg *msg);

  // ע��ص�����
  static void regMsgCallback() {
    regCb(cmsg::MSG_UPLOAD_CONFIG_REQ, (msgCbFunc)&CConfigHandle::uploadConfig);
    regCb(cmsg::MSG_DOWNLOAD_CONFIG_REQ, (msgCbFunc)&CConfigHandle::downloadConfig);
    regCb(cmsg::MSG_DOWNLOAD_ALL_CONFIG_REQ, (msgCbFunc)&CConfigHandle::downloadAllConfig);
    regCb(cmsg::MSG_DELETE_CONFIG_REQ, (msgCbFunc)&CConfigHandle::deleteConfig);
  }

private:

};



#endif

