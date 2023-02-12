#include "cregisterclient.h"
#include "cmsgcom.pb.h"
#include "ctools.h"

// ����ķ����б�
static cmsg::CServiceMap *pbServiceMap = nullptr;
// ���ػ���ķ����б�
static cmsg::CServiceMap *pbClientMap = nullptr;
static std::mutex pbServiceMapMtx;

// ���ӳɹ�����Ϣ�ص�
void CRegisterClient::connetedCb() {
  // ����ע����Ϣ
  LOG_DEBUG("ע�����Ŀͻ����ӳɹ��� ��ʼ����ע������!");
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

  // ��������뵽�̳߳���
  startConnect();
}

void CRegisterClient::getServiceReq(const char *serviceName) {
  LOG_DEBUG("������ȡ΢�����б������");
  cmsg::CGetServiceReq req;
  if (serviceName) {
    req.set_type(cmsg::CGetServiceReq_CType_ONE);
    req.set_name(serviceName);
  } else {
    req.set_type(cmsg::CGetServiceReq_CType_ALL);
  }
  sendMsg(cmsg::MSG_GET_SERVICE_REQ, &req);
}

cmsg::CServiceMap *CRegisterClient::getAllService() {
  std::lock_guard<std::mutex> guard(pbServiceMapMtx);
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
    LOG_DEBUG("�ȴ����ӳ�ʱ!");
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
  LOG_DEBUG("���յ�ע��������Ӧ");
  cmsg::CMessageRes res;
  if (!res.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("CRegisterClient::registerRes failed: CMessageRes ParseFromArray error!");
    return;
  }

  if (res.return_() == cmsg::CMessageRes::OK) {
    LOG_DEBUG("ע��΢����ɹ�");
      return;
  }

  std::stringstream ss;
  ss << "ע��΢����ʧ��: " << res.msg();
  LOG_DEBUG(ss.str().c_str());
}

void CRegisterClient::getServiceRes(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("���ջ�ȡ�����б����Ӧ");

  std::lock_guard<std::mutex> guard(pbServiceMapMtx);
  // ������洢������
  if (!pbServiceMap) {
    pbServiceMap = new cmsg::CServiceMap();
  }
  if (!pbServiceMap->ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("cmsg::CServiceMap ParseFromArray failed!");
    return;
  }
  LOG_DEBUG(pbServiceMap->DebugString());
}