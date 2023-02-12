#include "cservice.h"
#include "cdirservicehandle.h"
#include "crouterserver.h"
#include "cserviceproxy.h"
#include "cregisterclient.h"
#include <iostream>

int main(int argc, char *argv[]) {
  std::cout << "Run cmsgateway like this: ./[executable file name] [gateway port] [register ip] [register port]\n";

  int serverPort = API_GATEWAY_PORT;
  if (argc > 1) {
    serverPort = atoi(argv[1]);
  }

  std::cout << "server port is " << serverPort << '\n';
  std::string registerIp = "127.0.0.1";
  int registerPort = REGISTER_PORT;
  if (argc > 2) {
    registerIp = argv[2];
  }
  if (argc > 3) {
    registerPort = atoi(argv[3]);
  }


  // ����ע�����ĵ� IP �� �˿�
  CRegisterClient::get()->setServerIp(registerIp.c_str());
  CRegisterClient::get()->setServerPort(registerPort);
  // ע�ᵽע������
  CRegisterClient::get()->registerServer(API_GATEWAY_NAME, serverPort, 0);
  // �ȴ�ע����������
  CRegisterClient::get()->waitforConnected(3);
  CRegisterClient::get()->getServiceReq(0);

  // ��ʼ�� CServiceProxy���������Ӽ��뵽�̳߳���
  CServiceProxy::get()->init();
  // �����Զ�����
  CServiceProxy::get()->start();

  CRouterServer service;
  service.setServerPort(serverPort);
  service.start();
  CThreadPool::wait();

  return 0;
}