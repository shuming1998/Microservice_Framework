#include "cdirservicehandle.h"
#include "cconfigclient.h"
#include "ctools.h"
#include <iostream>

CDirServiceHandle::CDirServiceHandle() {}
CDirServiceHandle::~CDirServiceHandle() {}

void CDirServiceHandle::regMsgCb() {
  regCb(cmsg::MSG_DIR_REQ, (msgCbFunc)&CDirServiceHandle::dirReqCb);
}

void CDirServiceHandle::dirReqCb(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("CDirServiceHandle::dirReqCb");

  if (!head || !msg || !msg->data_ || msg->size_ <= 0) {
    LOG_DEBUG("!head || !msg || !msg->data_ || msg->size_ <= 0");
    return;
  }

  // ��Ӧ�ͻ���
  // ���ղ������л��ͻ���ָ��
  cmsg::CDirReq req;
  if (!req.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("req.ParseFromArray error");
    return;
  }

  std::cout << "client req path = " << req.path() << '\n';

  // ��Ӧ�ͻ��ˣ�ͷ����Ϣ����������·��
  cmsg::CDirRes res;
  std::string root = CConfigClient::get()->getString("root");


  ///// ���Դ��� /////
  static int count = 0;
  for (int i = 0; i < 10; ++i) {
    ++count;
    std::stringstream ss;
    ss << root << "/";
    ss << "filename " << count << "_" << i;
    auto dir = res.add_dirs();
    dir->set_filename(ss.str());
    dir->set_filesize((i + 1) * 1024);
  }

  head->set_msg_type(cmsg::MSG_DIR_RES);
  sendMsg(head, &res);
}