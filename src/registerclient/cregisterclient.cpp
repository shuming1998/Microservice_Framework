#include "cregisterclient.h"
#include "clogclient.h"
#include "cmsgcom.pb.h"
#include "ctools.h"
#include <fstream>
#include <thread>

// ����ķ����б�
static cmsg::CServiceMap *pbServiceMap = nullptr;
// ���ػ���ķ����б�
static cmsg::CServiceMap *pbClientMap = nullptr;
static std::mutex pbServiceMapMtx;

void CRegisterClient::timerCb() {
  static long long count = 0;
  count++;
  cmsg::CMsgHeart req;
  req.set_count(count);
  sendMsg(cmsg::MSG_HEART_REQ, &req);
}

// ���ӳɹ�����Ϣ�ص�
void CRegisterClient::connetedCb() {
  // ����ע����Ϣ
  LOG_DEBUG("CRegisterClient::connetedCb()");
  cmsg::CRegisterReq req;
  req.set_name(serviceName_);
  req.set_ip(serviceIp_);
  req.set_port(servicePort_);
  sendMsg(cmsg::MSG_REGISTER_REQ, &req);
}

void CRegisterClient::registerServer(const char *serviceName, int port, const char *ip) {
  // ע����Ϣ�ص�����
  regMsgCallback();
  // ������Ϣ��������
  // �����������Ƿ�ɹ� ��
  // ע�����ĵ�IP port
  if (serviceName) {
    strcpy(serviceName_, serviceName);
  }
  if (ip) {
    strcpy(serviceIp_, ip);
  }
  servicePort_ = port;

  // �����Զ�����
  setAutoConnect(true);

  // ����������ʱ��ʱ��
  setTimerMs(3000);

  // ���Ĭ�ϵ� IP �Ͷ˿�
  if (getServerIp()[0] == '\0') {
    setServerIp("127.0.0.1");
  }
  if (getServerPort() <= 0) {
    setServerPort(REGISTER_PORT);
  }

  // ��������뵽�̳߳���
  startConnect();
  loadLocalFile();
}

void CRegisterClient::getServiceReq(const char *serviceName) {
  LOG_DEBUG("CRegisterClient::getServiceReq");
  cmsg::CGetServiceReq req;
  if (serviceName) {
    req.set_type(cmsg::CServiceType::ONE);
    req.set_name(serviceName);
  } else {
    req.set_type(cmsg::CServiceType::ALL);
  }
  sendMsg(cmsg::MSG_GET_SERVICE_REQ, &req);
}

cmsg::CServiceMap *CRegisterClient::getAllService() {
  std::lock_guard<std::mutex> guard(pbServiceMapMtx);
  loadLocalFile();
  if (!pbServiceMap) {
    return nullptr;
  }
  if (!pbClientMap) {
    pbClientMap = new cmsg::CServiceMap();
  }
  pbClientMap->CopyFrom(*pbServiceMap);
  return pbClientMap;
}

cmsg::CServiceMap::CServiceList CRegisterClient::getServices(const char *serviceName, int timeoutSec) {
  cmsg::CServiceMap::CServiceList serviceList;

  // ʱ����������Ϊ 10 ����
  int totalTm = timeoutSec * 100;
  int curTm = 0;
  // 1 �ȴ����ӳɹ�
  while (curTm < totalTm) {
    if (isConnected()) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ++curTm;
  }

  if (!isConnected()) {
    LOG_DEBUG("wait for connect timeout!");
    // �����ӵȴ���ʱ��ʱ�򣬶�ȡ���ػ���
    // ֻ�е�һ�ζ�ȡ����
    std::lock_guard<std::mutex> guard(pbServiceMapMtx);
    if (!pbServiceMap) {
      loadLocalFile();
    }
    return serviceList;
  }

  // 2 ���ͻ�ȡ΢�������Ϣ
  getServiceReq(serviceName);

  // 3 �ȴ�΢�����б���Ϣ����(�п����õ���һ�ε�����)
  while (curTm < totalTm) {
    std::lock_guard<std::mutex> guard(pbServiceMapMtx);
    if (!pbServiceMap) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      ++curTm;
      continue;
    }
    auto pMap = pbServiceMap->mutable_servicemap();
    if (!pMap) {
      // û���ҵ�ָ����΢����
      getServiceReq(serviceName);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      curTm += 10;
      continue;
    }
    auto itServiceList = pMap->find(serviceName);
    if (itServiceList == pMap->end()) {
      // û���ҵ�ָ����΢����
      getServiceReq(serviceName);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      curTm += 10;
      continue;
    }
    serviceList.CopyFrom(itServiceList->second);
    return serviceList;
  }
  return serviceList;
}

void CRegisterClient::registerRes(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("CRegisterClient::registerRes");
  cmsg::CMessageRes res;
  if (!res.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("CRegisterClient::registerRes failed: CMessageRes ParseFromArray error!");
    return;
  }

  if (res.return_() == cmsg::CMessageRes::OK) {
    LOG_DEBUG("registe service success!");
      return;
  }

  std::stringstream ss;
  ss << "registe service failed: " << res.msg();
  LOG_DEBUG(ss.str().c_str());
}

void CRegisterClient::getServiceRes(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("CRegisterClient::getServiceRes");
  std::lock_guard<std::mutex> guard(pbServiceMapMtx);
  // �Ƿ��滻ȫ������
  bool isAll = false;
  cmsg::CServiceMap *cacheServiceMap;
  cmsg::CServiceMap tmpMap;
  cacheServiceMap = &tmpMap;
  // ������洢������
  if (!pbServiceMap) {
    pbServiceMap = new cmsg::CServiceMap();
    cacheServiceMap = pbServiceMap;
    isAll = true;
  }
  if (!cacheServiceMap->ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("cmsg::CServiceMap ParseFromArray failed!");
    return;
  }

  // �����ǻ�ȡһ�ֻ���ȫ�����ж����ˢ�»���
  if (cacheServiceMap->type() == cmsg::CServiceType::ALL) {
    isAll = true;
  }

  // �ڴ滺��ˢ��
  if (cacheServiceMap == pbServiceMap) {
    // �ڴ滺���Ѿ�ˢ��
  } else {
    if (isAll) {
      pbServiceMap->CopyFrom(*cacheServiceMap);
    } else {
      // ���ն�ȡ������ cacheMap �洢 pbServiceMap �ڴ滺����
      auto cacheMap = cacheServiceMap->mutable_servicemap();
      if (!cacheMap || cacheMap->empty()) {
        return;
      }
      // ֻȡ��һ��
      auto one = cacheMap->begin();
      auto sMap = pbServiceMap->mutable_servicemap();
      // �޸Ļ���
      (*sMap)[one->first] = one->second;
    }
  }

  // ���̻���ˢ�£�����Ӧ����ˢ��Ƶ�ʣ��ж�ˢ�²���(�Ƿ�����һ��������)
  // ��������
  std::stringstream ss;
  ss << "register_" << serviceName_ << serviceIp_ << servicePort_ << ".cache";
  LOG_DEBUG("Save local file!");
  if (!pbServiceMap) {
    return;
  }
  std::ofstream ofs;
  ofs.open(ss.str(), std::ios::binary);
  if (!ofs.is_open()) {
    LOG_DEBUG("save file failed!");
    return;
  }
  pbServiceMap->SerializePartialToOstream(&ofs);
  ofs.close();
}

bool CRegisterClient::loadLocalFile() {
  LOG_DEBUG("Load local register data");
  if (!pbServiceMap) {
    pbServiceMap = new cmsg::CServiceMap();
  }
  std::stringstream ss;
  ss << "register_" << serviceName_ << serviceIp_ << servicePort_ << ".cache";
  std::ifstream ifs;
  ifs.open(ss.str(), std::ios::binary);
  if (!ifs.is_open()) {
    std::stringstream log;
    log << "ParseFromIstream failed: [";
    log << ss.str();
    log << ']';
    LOG_DEBUG(log.str().c_str());
    return false;
  }

  pbServiceMap->ParseFromIstream(&ifs);
  ifs.close();
  return true;
}
