#include "cregisterserver.h"
#include "cregisterhandle.h"

CServiceHandle *CRegisterServer::createServiceHandle() {
  auto handle = new CRegisterHandle();
  // �趨��ʱ�����ڽ���������
  handle->setReadTimeoutMs(5000);
  return handle;
}

void CRegisterServer::mainFunc(int argc, char *argv[]) {
  // ע����Ϣ�ص�����
  CRegisterHandle::regMsgCallback();

  int port = REGISTER_PORT;
  if (argc > 1) {
    port = atoi(argv[1]);
  }
  // ���÷����������˿�
  setServerPort(port);
}

void CRegisterServer::wait() {
  CThreadPool::wait();
}