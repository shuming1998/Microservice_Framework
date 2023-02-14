#include "cservice.h"
#include "cregisterclient.h"
#include "cdirservicehandle.h"
#include "cconfigclient.h"
#include "ctools.h"
#include <thread>
#include <iostream>

class CTestService : public CService {
public:
  CServiceHandle *createServiceHandle() {
    return new CDirServiceHandle();
  }
};

int main(int argc, char *argv[]) {
  std::cout << "Run test_pbservice like this: ./[executable file name] [server port] [register ip] [register port]\n";
  int serverPort = 30001;
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
  CRegisterClient::get()->registerServer("dir", serverPort, 0);

  // �ҵ��������ĵ� IP �� �˿�
  // �ȴ����û�ȡ�ɹ�
  auto configs = CRegisterClient::get()->getServices(CONFIG_NAME, 10);
  std::cout << "=======================================\n";
  std::cout << configs.DebugString() << '\n';
  if (configs.service_size() <= 0) {
    std::cout << "find config service failed!\n";
  } else {
    // ֻȡ��һ����������
    auto conf = configs.service()[0];
    // ������������(��װ�� startGetConfig ��)
    //CConfigClient::get()->regMsgCallback();
    //CConfigClient::get()->setServerIp(conf.ip().c_str());
    //CConfigClient::get()->setServerPort(conf.port());
    //CConfigClient::get()->startConnect();
    //if (!CConfigClient::get()->waitforConnected(10)) {
    //  LOG_DEBUG("������������ʧ��!");
    //} else {
    //  LOG_DEBUG("�����������ĳɹ�!");
    //}

    static cmsg::CDirConfig dirConf1;
    if (CConfigClient::get()->startGetConfig(conf.ip().c_str(), 
          conf.port(), "127.0.0.1", serverPort, &dirConf1)) {
      std::cout << dirConf1.DebugString() << '\n';
    }

    // д����Ե�����
    cmsg::CConfig uploadConf;
    uploadConf.set_servicename("dir");
    uploadConf.set_serviceip("127.0.0.1");
    uploadConf.set_serviceport(serverPort);
    // ��������
    cmsg::CDirConfig dirConf; // �������Ե�����
    dirConf.set_root("./root");
    uploadConf.set_proto(dirConf.GetDescriptor()->DebugString());
    std::string dirConfPb = dirConf.SerializeAsString();
    uploadConf.set_privatepb(dirConfPb);
    CConfigClient::get()->uploadConfig(&uploadConf);

    CConfigClient::get()->downloadConfig("127.0.0.1", serverPort);
    // ���������Ŀͻ��˻�ȡ����
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    cmsg::CConfig tmpConfig;
    CConfigClient::get()->getConfig("127.0.0.1", serverPort, &tmpConfig);
    std::cout << "==============tmpConfig==============\n";
    std::cout << tmpConfig.DebugString() << '\n';

    // �����л����ص�������Ϣ
    cmsg::CDirConfig downloadConf;\
    // ÿ��΢���񵥶�������
    if (downloadConf.ParseFromString(tmpConfig.privatepb())) {
      std::cout << "downloadConf = " << downloadConf.DebugString() << '\n';
    }
 
  }


  // ע����Ϣ�ص�����
  CDirServiceHandle::regMsgCb();

  CTestService service;
  service.setServerPort(serverPort);
  service.start();
  CThreadPool::wait();

  return 0;
}