#include "cclientevent.h"
#include "cmsgcom.pb.h"
#include <iostream>
#include <thread>

std::map<cmsg::CMsgType, CClientEvent::msgCbFunc> CClientEvent::cbs_;

// ��ʼ���ص�����
void CClientEvent::init() {
  regCb(cmsg::MSG_LOGIN_RES, &CClientEvent::loginRes);
}


// ���յ�¼������Ϣ
// @param data ��Ϣ����
// @param size ��Ϣ��С
void CClientEvent::loginRes(const char *data, int size) {
  std::cout << "loginRes " << size << '\n';
  // ���װ �����л�
  cmsg::CLoginRes res;
  res.ParseFromArray(data, size);
  std::cout << res.res() << " recv server token: " << res.token() << '\n';

  // ���л�
  cmsg::CLoginReq req;
  char buf[1024] = { 0 };
  static int count = 0;
  ++count;
  sprintf(buf, "root_%d", count);
  req.set_username(buf);
  req.set_password("123456");
  sendMsg(cmsg::MSG_LOGIN_REQ, &req);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
}