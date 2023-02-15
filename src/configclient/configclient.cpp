#include "cconfigclient.h"
#include "cregisterclient.h"
#include "cmsgcom.pb.h"
#include "ctools.h"
#include <iostream>
#include <thread>
#include <string>

#define REG CRegisterClient::get()
#define CONF CConfigClient::get()


void configTimer() {
  static std::string configIp = "";
  static int configPort = 0;

  // ��ȡ������ //
  std::cout << "==========root = " << CONF->getString("root") << "==========\n";

  if (configPort <= 0) {
    // ��ע�����Ļ�ȡ�������ĵ� ip
    auto configs = REG->getServices(CONFIG_NAME, 1);
    std::cout << configs.DebugString() << '\n';
    if (configs.service_size() <= 0) {
      return;
    }
    auto config = configs.service()[0];
    if (config.ip().empty() || config.port() <= 0) {
      return;
    }
    configIp = config.ip();
    configPort = config.port();
    CONF->setServerIp(configIp.c_str());
    CONF->setServerPort(configPort);
    CONF->connect();
  }
}

int main(int argc, char *argv[]) {
  int clientPort = 4000;
  // ����ע�����ĵ� ip �� �˿�
  REG->setServerIp("127.0.0.1");
  REG->setServerPort(REGISTER_PORT);
  // ���Լ�ע�ᵽע������
  REG->registerServer("test_config", clientPort, 0);
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  // ��ʼ����������
  cmsg::CDirConfig tmpConf;
  //CONF->startGetConfig(config.ip().c_str(), config.port(), 0, clientPort, &tmpConf);
  CONF->startGetConfig(0, clientPort, &tmpConf, configTimer);
  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  // �洢������ //
  std::string proto;
  // ͨ���������� msg
  auto msg = CONF->loadProto("cmsgcom.proto", "CDirConfig", proto);
  // ͨ����������ֵ
  auto ref = msg->GetReflection();
  auto field = msg->GetDescriptor()->FindFieldByName("root");
  ref->SetString(msg, field, "test_new_root");
  std::cout << msg->DebugString();

  // �洢����
  cmsg::CConfig conf;
  conf.set_servicename("test_config");
  conf.set_serviceport(clientPort);
  conf.set_proto(proto);
  conf.set_privatepb(msg->SerializePartialAsString());
  CONF->uploadConfig(&conf);

  // ��ȡ������ //
  //std::cout << "=====root = " << CONF->getString("root") << "=====\n";

  // ��ȡ�����б�(������ʹ��) //
  for (int i = 0; i < 1; ++i) {
    // ��ȡ�����б�
    auto confs = CONF->downloadAllConfig(1, 1000, 10);
    std::cout << "=====================================" << '\n';
    std::cout << confs.DebugString() << '\n';
    if (confs.config_size() <= 0) {
      std::this_thread::sleep_for(std::chrono::seconds(2));
      continue;
    }

    // ȡ�õ���������Ϣ(��һ��������)
    std::string ip = confs.config()[0].serviceip();
    int port = confs.config()[0].serviceport();
    CONF->downloadConfig(ip.c_str(), port);
    cmsg::CConfig saveConf;
    CONF->getConfig(ip.c_str(), port, &saveConf);
    std::cout << "11111111111111111111111111111111111" << '\n';
    std::cout << saveConf.DebugString() << '\n';
    std::this_thread::sleep_for(std::chrono::seconds(2));
  }

  //CConfigClient::get()->regMsgCallback();
  //CConfigClient::get()->setServerIp("127.0.0.1");
  //CConfigClient::get()->setServerPort(CONFIG_PORT);
  //CConfigClient::get()->startConnect();
  //if (!CConfigClient::get()->waitforConnected(10)) {
  //  LOG_DEBUG("������������ʧ��!");
  //} else {
  //  LOG_DEBUG("�����������ĳɹ�!");
  //}

  //cmsg::CConfig conf;
  //conf.set_servicename("test_client_name");
  //conf.set_serviceip("127.0.0.1");
  //conf.set_serviceport(20030);
  //conf.set_proto("message");
  //conf.set_privatepb("test pb");
  //CConfigClient::get()->uploadConfig(&conf);
  //CConfigClient::get()->downloadConfig("127.0.0.1", 20030);

  //// ���������Ŀͻ��˻�ȡ����
  //std::this_thread::sleep_for(std::chrono::milliseconds(300));
  //cmsg::CConfig tmpConfig;
  //CConfigClient::get()->getConfig("127.0.0.1", 20030, &tmpConfig);
  //std::cout << "==============tmpConfig==============\n";
  //std::cout << tmpConfig.DebugString() << '\n';

  //// ���������Ļ�ȡ��������
  //auto configs = CConfigClient::get()->downloadAllConfig(1, 2, 10);
  //std::cout << configs.DebugString() << '\n';
  CConfigClient::get()->wait();
  return 0;
}