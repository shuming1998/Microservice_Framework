#include "CFtpServerCmd.h"
#include <iostream>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <string.h>

// event �¼��ص�����
void eventCb(bufferevent *bev, short flag, void *arg) {
  // ����ͻ�������Ͽ������������п����޷��յ� BEV_EVENT_EOF ����
  if ((flag & BEV_EVENT_EOF) | (flag & BEV_EVENT_ERROR) | (flag & BEV_EVENT_TIMEOUT)) {
    std::cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR\n";
    bufferevent_free(bev);
    delete (CFtpServerCmd *)arg;
  }
}

// �����߳� CThread event �¼��ַ��е���
static void readCb(bufferevent *bev, void *arg) {
  char buf[1024] = { 0 };
  for (;;) {
    int len = bufferevent_read(bev, buf, sizeof(buf) - 1);
    if (len <= 0) {
      break;
    }
    buf[len] = '\0';
    std::cout << buf;

    // ����
    if (strstr(buf, "quit")) {
      bufferevent_free(bev);
      delete (CFtpServerCmd *)arg;
      break;
    }
  }
}

// ��ʼ������ ���������߳���
bool CFtpServerCmd::init() {
  std::cout << "CFtpServerCmd::init()\n";

  // ���� bufferevent ���� socket
  // ���� bufferevent
  bufferevent *bev = bufferevent_socket_new(base_, sock_, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(bev, readCb, 0, eventCb, this);
  bufferevent_enable(bev, EV_READ | EV_WRITE);

  // ��ӳ�ʱ����
  timeval readTv = { 10, 0 }; // ���¼��ĳ�ʱ, 10 ��
  bufferevent_set_timeouts(bev, &readTv, 0);

  return true;
} 