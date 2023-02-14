#include "cconfigclient.h"
#include "cmsgcom.pb.h"
#include "ctools.h"
#include <iostream>
#include <thread>

int main(int argc, char *argv[]) {
  CConfigClient::get()->regMsgCallback();
  CConfigClient::get()->setServerIp("127.0.0.1");
  CConfigClient::get()->setServerPort(CONFIG_PORT);
  CConfigClient::get()->startConnect();
  if (!CConfigClient::get()->waitforConnected(10)) {
    LOG_DEBUG("������������ʧ��!");
  } else {
    LOG_DEBUG("�����������ĳɹ�!");
  }

  cmsg::CConfig conf;
  conf.set_servicename("test_client_name");
  conf.set_serviceip("127.0.0.1");
  conf.set_serviceport(20030);
  conf.set_proto("message");
  conf.set_privatepb("test pb");
  CConfigClient::get()->uploadConfig(&conf);
  CConfigClient::get()->downloadConfig("127.0.0.1", 20030);

  // ���������Ŀͻ��˻�ȡ����
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  cmsg::CConfig tmpConfig;
  CConfigClient::get()->getConfig("127.0.0.1", 20030, &tmpConfig);
  std::cout << "==============tmpConfig==============\n";
  std::cout << tmpConfig.DebugString() << '\n';

  // ���������Ļ�ȡ��������
  auto configs = CConfigClient::get()->downloadAllConfig(1, 2, 10);
  std::cout << configs.DebugString() << '\n';


  CConfigClient::get()->wait();
  return 0;
}