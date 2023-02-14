#include "cregisterclient.h"
#include "ctools.h"
#include <iostream>
#include <string>
#include <thread>


int main(int argc, char *argv[]) {
  std::cout << "Run RegisterClient like this: ./[executable file name]  [ip]  [port]\n";
  // ����ע������ IP �˿ں�
  std::string ip = "127.0.0.1";
  int port = REGISTER_PORT;
  if (argc > 2) {
    ip = argv[1];
    port = atoi(argv[2]);
  }
  CRegisterClient::get()->setServerIp(ip.c_str());
  CRegisterClient::get()->setServerPort(port);
  CRegisterClient::get()->registerServer("test", 20020, 0);
  // �ȴ����룬ȷ�����㹻ʱ������
  CRegisterClient::get()->waitforConnected(3);

  // ����ֻע��
  CThreadPool::wait();

  // ������ȡȫ�����������
  // CRegisterClient::get()->getServiceReq(NULL);
  // CRegisterClient::get()->getServiceReq("test");
  for (;;) {
    CRegisterClient::get()->getServiceReq(NULL);
    CRegisterClient::get()->getServiceReq("test");
    auto services = CRegisterClient::get()->getAllService();
    if (services) {
      LOG_DEBUG(services->DebugString());
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  CThreadPool::wait();

  return 0;
}