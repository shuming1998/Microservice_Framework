#include "cserviceproxy.h"
#include "cmsgcom.pb.h"
#include "cregisterclient.h"
#include "ctools.h"
#include "clogclient.h"
#include <thread>

// �����Զ��������߳�
void CServiceProxy::start() {
  std::thread t(&CServiceProxy::mainFunc, this);
  t.detach();
}

// ֹͣ�߳�
void CServiceProxy::stop() {
   
}

// �Զ�����
void CServiceProxy::mainFunc() {
  // ��ע�����Ļ�ȡ΢������б�


  while (!isExit_) {
    // ��ע�����Ļ�ȡ΢������б����
    // ��������ע������
    CRegisterClient::get()->getServiceReq(0);

    auto serviceMap = CRegisterClient::get()->getAllService();
    if (!serviceMap) {
      LOG_DEBUG("CRegisterClient::get()->getAllService(): service is null!");
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }

    auto pbMap = serviceMap->servicemap();
    if (pbMap.empty()) {
      LOG_DEBUG("servicemap is empty!");
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // �������е�΢�������Ƶ��б�
    for (auto pbServiceMap : pbMap) {
      // ����ÿ���б��е�΢����
      std::string serviceName = pbServiceMap.first;
      // �������Լ�
      if (serviceName == API_GATEWAY_NAME) {
        continue;
      }
      for (auto pbService : pbServiceMap.second.service()) {
        std::lock_guard<std::mutex> guard(clientMapMtx_);
        // �����΢�������Ʋ����ڣ��½�һ��΢�����б�
        if (clientMap_.find(serviceName) == clientMap_.end()) {
          clientMap_[serviceName] = std::vector<CServiceProxyClient *>();
        }

        bool isFind = false;
        // �鿴�÷������ƶ�Ӧ���б����Ƿ��Ѿ��и� ip:port ע���΢����
        for (auto connect : clientMap_[serviceName]) {
          if (pbService.ip() == connect->getServerIp() && pbService.port() == connect->getServerPort()) {
            isFind = true;
            break;
          }
        }

        if (isFind) {
          continue;
        }

        // �����ǰ��΢����û�����б��ʹ���һ����΢����Ķ��󣬽������ӣ�����΢�����б�
        auto proxy = new CServiceProxyClient();
        proxy->setServerIp(pbService.ip().c_str());
        proxy->setServerPort(pbService.port());
        proxy->setAutoDelete(false);
        proxy->startConnect();
        clientMap_[serviceName].push_back(proxy);
        clientMapLastIdx_[serviceName] = 0;
      }
    }

    // ��ʱȫ�����»�ȡ
    for (auto m : clientMap_) {
      for (auto c : m.second) {
        if (c->isConnected()) {
          continue;
        }
        if (!c->isConnecting()) {
          LOG_DEBUG("start connect service");
          c->connect();
        }
      }
    }
    std::this_thread::sleep_for(std::chrono::seconds(3));
  }
}

bool CServiceProxy::init() {
  // 1 ��ע�����Ļ�ȡ΢�����б�

  //// ��������
  //cmsg::CServiceMap serviceMap;
  //// ��ȡ serviceMap ��ָ��
  //auto sMap = serviceMap.mutable_servicemap();
  //cmsg::CServiceMap::CServiceList list;
  //{
  //  auto service = list.add_service();
  //  service->set_ip("127.0.0.1");
  //  service->set_port(20011);
  //  service->set_name("dir");
  //}

  //{
  //  auto service = list.add_service();
  //  service->set_ip("127.0.0.1");
  //  service->set_port(20012);
  //  service->set_name("dir");
  //}

  //{
  //  auto service = list.add_service();
  //  service->set_ip("127.0.0.1");
  //  service->set_port(20013);
  //  service->set_name("dir");
  //}

  //(*sMap)["dir"] = list;

  //std::cout << serviceMap.DebugString() << '\n';

  //// 2 ��΢����������
  //// ���� serviceMap ����
  //for (auto m : (*sMap)) {
  //  clientMap_[m.first] = std::vector<CServiceProxyClient *>();
  //  for (auto s : m.second.service()) {
  //    auto proxy = new CServiceProxyClient();
  //    proxy->setServerIp(s.ip().c_str());
  //    proxy->setServerPort(s.port());
  //    proxy->startConnect();
  //    clientMap_[m.first].push_back(proxy);
  //    clientMapLastIdx_[m.first] = 0;
  //  }
  //}




  return true;
}

void CServiceProxy::delEvent(CMsgEvent *ev) {
  if (!ev) {
    return;
  }
  std::lock_guard<std::mutex> guard(clientCbsMtx_);
  auto it = clientCbs_.find(ev);
  if (it == clientCbs_.end()) {
    LOG_DEBUG("can't find ev in clientCbs_!");
  }
  it->second->delEvent(ev);
}

bool CServiceProxy::sendMsg(cmsg::CMsgHead *head, CMsg *msg, CMsgEvent *ev) {
  if (!head || !msg) {
    return false;
  }
  std::string serviceName = head->service_name();
  // 1 ���ؾ����ҵ��ͻ�������
  std::lock_guard<std::mutex> guard(clientMapMtx_);
  auto clientList = clientMap_.find(serviceName);
  if (clientList == clientMap_.end()) {
    std::stringstream ss;
    ss << serviceName << " not find int clientMap_!";
    LOG_DEBUG(ss.str().c_str());
    return false;
  }

  // ��ѯ�ҵ����õ�΢��������
  int curIdx = clientMapLastIdx_[serviceName];
  int listSize = clientList->second.size();
  for (int i = 0; i < listSize; ++i) {
    ++curIdx;
    curIdx = curIdx % listSize;
    clientMapLastIdx_[serviceName] = curIdx;
    auto client = clientList->second[curIdx];
    if (client->isConnected()) {
      
    {
      // ���������˳�ʱ����ص���������
      std::lock_guard<std::mutex> guard(clientCbsMtx_);
      clientCbs_[ev] = client;
    }

      // ת����Ϣ
      return client->sendMsg(head, msg, ev);
    }
  }

  LOG_DEBUG("can't find proxy");

  return false;
}