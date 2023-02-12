#ifndef CTEST_CLIENT_H
#define CTEST_CLIENT_H

#include "cserviceclient.h"

class CTestClient : public CServiceClient {
public:

  //void readCb();
  //void connectedCb();

  // ע��ص�����
  static void regMsgCb() {
    regCb(cmsg::MSG_DIR_RES, (msgCbFunc)&dirResCb);
  }

  // ��ȡĿ¼
  bool getDir(std::string path);

  // ����Զ�����������ʧ��ʱ���̷���
  bool autoReconnect(int timeoutMs);

  // getDir Ŀ¼�������Ӧ
  void dirResCb(cmsg::CMsgHead *head, CMsg *msg);

private:

};


#endif // !CTEST_CLIENT_H
