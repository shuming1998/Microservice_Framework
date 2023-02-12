#include "cmsgserver.h"
#include "cmsgevent.h"
#include "cmsgcom.pb.h"
#include "cserverevent.h"
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#ifdef _WIN32
#else
#include <signal.h>
#endif // _WIN32

static void readCb(struct bufferevent* bev, void* ctx) {
  std::cout << "server readCb:\n" << std::flush;
  auto ev = (CServerEvent *)ctx;
  // �ж���Ϣ�����Ƿ�ɹ���ʧ����Ҫ�ͷ���Դ
  if (!ev->recvMsg()) {
    ev->clear();
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
  }
}

static void listenCb(struct evconnlistener* evc, evutil_socket_t client_socket, struct sockaddr* clientAddr, int socklen, void* args) {
  char ip[16] = { 0 };

  sockaddr_in* addr = (sockaddr_in*)clientAddr;
  evutil_inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
  std::cout << "client ip is: " << ip << '\n';

  // ���� bufferevent ���� bev��read �� write)��ͨ�� bev ��������ͨ��
  event_base* base = (event_base*)args;
  // BEV_OPT_CLOSE_ON_FREE �ر� BEV ʱ�ر� socket
  bufferevent* bev = bufferevent_socket_new(base, client_socket, BEV_OPT_CLOSE_ON_FREE);
  if (!bev) {
    std::cerr << "bufferevent_socket_new failed!\n";
    return;
  }

  // bufferevent_socket_new ���Ѿ������˶���д�� event��Ȼ����Ӽ���¼��������ڲ�Ȩ�޲���
  bufferevent_enable(bev, EV_READ | EV_WRITE);
  // ���ó�ʱʱ��: �룬΢��(1/1000000)   ����ʱ��д��ʱ
  // �ͻ������Ӻ� 10 ���ڲ�����Ϣ�ͻᳬʱ
  timeval tl = { 30, 0 };
  bufferevent_set_timeouts(bev, &tl, 0);

  auto ev = new CServerEvent();
  ev->setBev(bev);

  // ���ûص�����
  bufferevent_setcb(bev, readCb, 0, eventCb, ev);
}

void CMsgServer::init(int serverPort) {
  // ע����Ϣ�ص�����
  CServerEvent::init();
  // ���� libevent �������ģ�Ĭ�ϴ��� base ��
  event_base* base = event_base_new();
  if (base) {
    std::cout << "event_base_new success!\n";
  }

  // ��������
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(serverPort);
  auto evc = evconnlistener_new_bind(base, listenCb, base, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, 10, (sockaddr*)&addr, sizeof(addr));

  // �¼���ѭ���������ж��¼��Ƿ������Լ��ַ��¼����ص�����
  event_base_dispatch(base);
  // ���û���¼�ע����˳���������Ҫ������Դ
  evconnlistener_free(evc);
  event_base_free(base);
}