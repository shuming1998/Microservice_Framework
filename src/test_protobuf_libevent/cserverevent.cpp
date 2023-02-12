#include "cserverevent.h"
#include "cmsgcom.pb.h"
#include <iostream>

std::map<cmsg::CMsgType, CServerEvent::msgCbFunc> CServerEvent::cbs_;

// ��ʼ���ص�����
void CServerEvent::init() {
  regCb(cmsg::MSG_LOGIN_REQ, &CServerEvent::loginReq);
}

// ���յ�¼������Ϣ
// @param data ��Ϣ����
// @param size ��Ϣ��С
void CServerEvent::loginReq(const char *data, int size) {

  // �����л���Ϣ
  cmsg::CLoginReq req;
  req.ParseFromArray(data, size);
  std::cout << "recv user name = " << req.username() << '\n';
  std::cout << "recv password = " << req.password() << '\n';

  // ������Ϣ
  cmsg::CLoginRes res;
  res.set_res(cmsg::CLoginRes::OK);
  std::string token = req.username();
  token += "sign";
  res.set_token(token);
  sendMsg(cmsg::MSG_LOGIN_RES, &res);
}