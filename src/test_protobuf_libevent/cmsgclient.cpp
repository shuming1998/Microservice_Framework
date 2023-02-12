#include "cmsgclient.h"
#include "cmsgevent.h"
#include "cclientevent.h"
#include "cmsgcom.pb.h"
#include "cmsgtype.pb.h"
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <iostream>
#include <thread>
#include <chrono>

static void readCb(struct bufferevent* bev, void* ctx) {
  std::cout << "client readCb:\n" << std::flush;

  auto ev = (CClientEvent *)ctx;
  // �ж���Ϣ�����Ƿ�ɹ���ʧ����Ҫ�ͷ���Դ
  if (!ev->recvMsg()) {
    delete ev;
    bufferevent_free(bev);
    return;
  }

  auto msg = ev->getMsg();
  if (!msg) {
    return;
  }
  ev->callFunc(msg->type_, msg->data_, msg->size_);
  ev->clear();
}

static void eventCb(struct bufferevent* bev, short what, void* ctx) {
  std::cout << "server eventCb\n" << '\n';
  auto ev = (CMsgEvent *)ctx;
  // ����ʱ
  if (what & BEV_EVENT_TIMEOUT || what & BEV_EVENT_ERROR || what & BEV_EVENT_EOF) {
    std::cout << "BEV_EVENT_READING || BEV_EVENT_ERROR || BEV_EVENT_EOF\n";
    // ��Ҫ���жϻ������Ƿ�������Ϊ����
    delete ev;
    // ����ռ䣬�رռ���
    bufferevent_free(bev);
    return;
  }

  if (what & BEV_EVENT_CONNECTED) {
    std::cout << "BEV_EVENT_CONNECTED\n";
    // bufferevent_write(bev, "ok", 3);
    // ���������¼��Ϣ
    cmsg::CLoginReq req;
    req.set_username("root");
    req.set_password("123456");
    ev->sendMsg(cmsg::MSG_LOGIN_REQ, &req);
  }
}

void CMsgClient::startThread() {
  std::thread t(&CMsgClient::mainFunc, this);
  t.detach();
}

void CMsgClient::mainFunc() {
  // ע����Ϣ�ص�����
  CClientEvent::init();
  if (serverPort_ <= 0) {
    std::cerr << "client error: please set server port!\n";
  }
  std::this_thread::sleep_for(std::chrono::microseconds(200));  // �ȴ����������
  std::cout << "CMsgClient::mainFunc() start!\n";

  event_base *base = event_base_new();
  // ���ӷ����
  bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
  
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(this->serverPort_);
  if (this->serverIp_.empty()) {
    serverIp_ = "127.0.0.1";
  }
  evutil_inet_pton(AF_INET, this->serverIp_.c_str(), &addr.sin_addr.s_addr);

  // ���ûص�����
  // bufferevent_socket_new ���Ѿ������˶���д�� event��Ȼ����Ӽ���¼��������ڲ�Ȩ�޲���
  bufferevent_enable(bev, EV_READ | EV_WRITE);
  // ���ó�ʱʱ��: �룬΢��(1/1000000)   ����ʱ��д��ʱ
  // �ͻ������Ӻ� 10 ���ڲ�����Ϣ�ͻᳬʱ
  timeval tl = { 30, 0 };
  bufferevent_set_timeouts(bev, &tl, 0);

  auto ev = new CClientEvent();
  ev->setBev(bev);

  // ���ûص�����
  bufferevent_setcb(bev, readCb, 0, eventCb, ev);

  int res = bufferevent_socket_connect(bev, (sockaddr *)&addr, sizeof(addr));
  if (res != 0) {
    std::cerr << "bufferevent_socket_connect error!\n";
    return;
  }

  // �¼���ѭ���������ж��¼��Ƿ������Լ��ַ��¼����ص�����
  event_base_dispatch(base);
  event_base_free(base);
}
