#include "cconfigserver.h"
#include "configdao.h"
#include "cmsgcom.pb.h"
#include "ctools.h"
#include <iostream>

int main(int argc, char *argv[]) {
  std::cout << "===== config server =====\n";


  // ��ʼ�����ݿ�
  if (ConfigDAO::get()->init("127.0.0.1", "root", "123456", "cms", 3306)) {
    std::cout << "ConfigDAO::get()->init Success!\n";
    ConfigDAO::get()->install();
  }


  auto res = ConfigDAO::get()->downloadAllConfig(1, 10000);
  std::cout << res.DebugString() << '\n';

  CConfigServer configServer;
  configServer.mainFunc(argc, argv);
  configServer.start();
  configServer.wait();

  //// ���� DAO
  //if (ConfigDAO::get()->init("127.0.0.1", "root", "123456", "cms", 3306)) {
  //  std::cout << "ConfigDAO::get()->init Success!\n";
  //  // ���������ݿ��а�װ��
  //  ConfigDAO::get()->install();

  //  // ���Բ���һ������
  //  cmsg::CConfig conf;
  //  conf.set_servicename("test1");
  //  conf.set_serviceip("127.0.0.1");
  //  conf.set_serviceport(20020);
  //  conf.set_proto("message Test{string name = 1;}");
  //  std::string pb = conf.SerializeAsString();
  //  conf.set_privatepb(pb);

  //  ConfigDAO::get()->uploadConfig(&conf);

  //  // ������
  //  auto config = ConfigDAO::get()->downloadConfig("127.0.0.1", 20020);
  //  std::cout << "=======================================\n";
  //  LOG_DEBUG(config.DebugString());
  //} else {
  //  std::cout << "ConfigDAO::get()->init Failed!\n";
  //}
  //getchar();


  return 0;
}