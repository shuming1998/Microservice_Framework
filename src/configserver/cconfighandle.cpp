#include "cconfighandle.h"
#include "configdao.h"
#include "ctools.h"

void CConfigHandle::uploadConfig(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("���յ��������õ���Ϣ");

  // ��Ӧ��Ϣ
  cmsg::CMessageRes res;

  // �ӽ��յ�����Ϣ�з����л�����Ϣ����
  cmsg::CConfig conf;
  if (!conf.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("CConfigHandle::uploadConfig ParseFromArray failed!");
    res.set_return_(cmsg::CMessageRes_CReturn_ERROR);
    res.set_msg("format error!");
    sendMsg(cmsg::MSG_UPLOAD_CONFIG_RES, &res);
    return;
  }

  if (conf.serviceip().empty()) {
    std::string ip = clientIp();
    conf.set_serviceip(ip);
  }

  if (ConfigDAO::get()->uploadConfig(&conf)) {
    res.set_return_(cmsg::CMessageRes_CReturn_OK);
    res.set_msg("OK");
    sendMsg(cmsg::MSG_UPLOAD_CONFIG_RES, &res);
    return;
  }

  res.set_return_(cmsg::CMessageRes_CReturn_ERROR);
  res.set_msg("insert to db failed!");
  sendMsg(cmsg::MSG_UPLOAD_CONFIG_RES, &res);
}

void CConfigHandle::downloadConfig(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("���յ��������õ���Ϣ");
  cmsg::CDownloadconfigReq req;

  if (!req.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("downloadConfig ParseFromArray failed!");
    return;
  }

  // ��� ip Ϊ�գ�ʹ�ÿͻ��� ip
  std::string ip = req.serviceip();
  if (ip.empty()) {
    ip = clientIp();
  }

  // ���� ip �� port ��ȡ�����Ȼ���͸��ͻ���
  cmsg::CConfig conf = ConfigDAO::get()->downloadConfig(ip.c_str(), req.serviceport());
  sendMsg(cmsg::MSG_DOWNLOAD_CONFIG_RES, &conf);

}

void CConfigHandle::downloadAllConfig(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("���յ������з�ҳ��ȫ�����õ���Ϣ");
  cmsg::CDownloadAllConfigReq req;
  if (!req.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("downloadAllConfig ParseFromArray failed!");
    return;
  }

  auto configs = ConfigDAO::get()->downloadAllConfig(req.page(), req.pagecount());
  // ���͸��ͻ���
  sendMsg(cmsg::MSG_DOWNLOAD_ALL_CONFIG_RES, &configs);
}

void CConfigHandle::deleteConfig(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("���յ�ɾ��ָ��΢�������õ���Ϣ");
  // ��Ӧ��Ϣ
  cmsg::CMessageRes res;
  cmsg::CDownloadconfigReq req;

  if (!req.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("deleteConfig ParseFromArray failed!");
    return;
  }

  if (ConfigDAO::get()->deleteConfig(req.serviceip().c_str(), req.serviceport())) {
    res.set_return_(cmsg::CMessageRes_CReturn_OK);
    res.set_msg("OK");
    sendMsg(cmsg::MSG_DELETE_CONFIG_RES, &res);
    return;
  }

  res.set_return_(cmsg::CMessageRes_CReturn_ERROR);
  res.set_msg("ConfigDAO::get()->deleteConfig failed!");
  sendMsg(cmsg::MSG_DOWNLOAD_CONFIG_RES, &res);
}
