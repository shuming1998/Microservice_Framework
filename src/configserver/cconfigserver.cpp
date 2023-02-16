#include "cconfigserver.h"
#include "cregisterclient.h"
#include "cconfighandle.h"
#include "clogclient.h"
#include "ctools.h"

CServiceHandle *CConfigServer::createServiceHandle() {
  return new CConfigHandle();
}

void CConfigServer::mainFunc(int argc, char *argv[]) {
  // ע�����ĵ�����
  LOG_DEBUG("run configServer like: ./[executable file name] [register ip] [register port] [configServer port]");

  // ע��ص�����
  CConfigHandle::regMsgCallback();

  int servicePort = CONFIG_PORT;
  int registerPort = REGISTER_PORT;
  std::string registerIp = "127.0.0.1";
  if (argc > 1) {
    registerIp = argv[1];
  }
  if (argc > 2) {
    registerPort = atoi(argv[2]);
  }
  if (argc > 3) {
    servicePort = atoi(argv[3]);
  }

  // ���÷����������˿�
  setServerPort(servicePort);


  CRegisterClient::get()->setServerIp(registerIp.c_str());
  CRegisterClient::get()->setServerPort(registerPort);

  // ����������ע������ע��
  CRegisterClient::get()->registerServer(CONFIG_NAME, servicePort, 0);
}

void CConfigServer::wait() {
  CThreadPool::wait();
}