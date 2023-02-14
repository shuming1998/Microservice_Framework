#include "cregisterhandle.h"
#include "cmsgcom.pb.h"
#include "ctools.h"

// ע������б�Ļ��棬ʹ��ָ������ڽ��� main ����֮ǰ��ʼ�����󲻺��Ų����
static cmsg::CServiceMap *serviceMap = nullptr;
// ���ڻ������ 
static std::mutex serviceMapMtx;

void CRegisterHandle::registerReq(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("CRegisterHandle::registerReq");

  // ��Ӧ����Ϣ
  cmsg::CMessageRes res;
  // ��������
  cmsg::CRegisterReq req;
  std::string error;
  if (!req.ParseFromArray(msg->data_, msg->size_)) {
    error = "CRegisterReq ParseFromArray failed!";
    LOG_DEBUG(error.c_str());
    res.set_return_(cmsg::CMessageRes::ERROR);
    res.set_msg(error);
    sendMsg(cmsg::MSG_REGISTER_RES, &res);
    return;
  }

  // ���յ��û��ķ������ơ�IP���˿ں�
  std::string serviceName = req.name();
  if (serviceName.empty()) {
    error = "serviceName is empty!";
    LOG_DEBUG(error.c_str());
    res.set_return_(cmsg::CMessageRes::ERROR);
    res.set_msg(error);
    sendMsg(cmsg::MSG_REGISTER_RES, &res);
    return;
  }

  std::string serviceIp = req.ip();
  if (serviceIp.empty()) {
    // �������δ���� ip ��ֱ������Ϊ�ͻ��˵� ip
    LOG_DEBUG("serviceIp is empty: use client ip!");
    serviceIp = this->clientIp();
  }

  int servicePort = req.port();
  if (servicePort <= 0 || servicePort >= 65536) {
    std::stringstream ss;
    ss << "invalid servicePort�� " << servicePort << " out of range: 1 ~ 65535!";
    LOG_DEBUG(ss.str().c_str());
    res.set_return_(cmsg::CMessageRes::ERROR);
    res.set_msg(ss.str());
    sendMsg(cmsg::MSG_REGISTER_RES, &res);
    return;
  }

  // ����ȷ�����û�ע����Ϣ
  std::stringstream ss;
  ss << "registerReq: " << serviceName << "->" << serviceIp << ":" << servicePort;
  LOG_INFO(ss.str().c_str());

  // �洢�û�ע����Ϣ������Ѿ�ע��͸�����Ϣ(��������)
  {
    std::lock_guard<std::mutex> guard(serviceMapMtx);
    if (!serviceMap) {
      serviceMap = new cmsg::CServiceMap();
    }

    // ��ȡ map ��ָ��
    auto pbMap = serviceMap->mutable_servicemap();
    // �ж� map ���Ƿ��Ѿ�ע��ͬ����΢����
    // ��Ⱥ΢���񣬻�ȡ������΢������б�
    auto serviceList = pbMap->find(serviceName);
    if (serviceList == pbMap->end()) {
      // ��΢����δע���
      (*pbMap)[serviceName] = cmsg::CServiceMap::CServiceList();
      serviceList = pbMap->find(serviceName);

    } // lock_guard
    // �Ѿ�ע����������Ƿ�����ͬ ip �� port ��΢����
    // ��ȡ pb service �б��ָ��
    auto pbServiceRted = serviceList->second.mutable_service();
    // ���� pb service �б��е� service
    for (auto service : (*pbServiceRted)) {
      if (service.ip() == serviceIp && service.port() == servicePort) {
        std::stringstream ss;
        ss << "service [" << serviceName.c_str() << "->" << serviceIp << ':' << servicePort << "] already registed!";
        LOG_DEBUG(ss.str().c_str());
        res.set_return_(cmsg::CMessageRes::ERROR);
        res.set_msg(ss.str());
        sendMsg(cmsg::MSG_REGISTER_RES, &res);
        return;
      }
    }
    // ΢����û��ע�ᣬ����µ�΢����
    auto pService = serviceList->second.add_service();
    pService->set_ip(serviceIp);
    pService->set_port(servicePort);
    pService->set_name(serviceName);
    std::stringstream ss;
    ss << "service [" << serviceName.c_str() << "->" << serviceIp << ':' << servicePort << "] registe success!";
    LOG_DEBUG(ss.str().c_str());
  } // lock_guard

  res.set_return_(cmsg::CMessageRes::OK);
  res.set_msg("OK");
  sendMsg(cmsg::MSG_REGISTER_RES, &res);
}

void CRegisterHandle::getServiceReq(cmsg::CMsgHead *head, CMsg *msg) {
  // ��ʱֻ����ȫ������
  LOG_DEBUG("CRegisterHandle::getServiceReq");

  cmsg::CGetServiceReq req;

  // ���ڴ�����
  cmsg::CServiceMap res;
  res.mutable_res()->set_return_(cmsg::CMessageRes_CReturn_ERROR);

  if (!req.ParseFromArray(msg->data_, msg->size_)) {
    std::stringstream ss;
    ss << "req.ParseFromArray failed!";
    LOG_DEBUG(ss.str().c_str());
    res.mutable_res()->set_msg(ss.str().c_str());
    sendMsg(cmsg::MSG_GET_SERVICE_RES, &res);
    return;
  }

  // ���ص��ֻ���ȫ��
  serviceMap->set_type(req.type());

  std::string serviceName = req.name();
  std::stringstream ss;
  ss << "CRegisterHandle::getServiceReq service name: " << serviceName;
  LOG_DEBUG(ss.str().c_str());

  // ����ȫ��΢��������
  {
    std::lock_guard<std::mutex> guard(serviceMapMtx);
    serviceMap->mutable_res()->set_return_(cmsg::CMessageRes_CReturn_OK);
    sendMsg(cmsg::MSG_GET_SERVICE_RES, serviceMap);
  }
}