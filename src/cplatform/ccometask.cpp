#include "ccometask.h"
#include "ctools.h"
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <iostream>
#include <string.h>

CComeTask::CComeTask() {
  mtx_ = new std::mutex();
}

CComeTask::~CComeTask() {
  delete mtx_;
}

static void sReadCb(struct bufferevent *bev, void *ctx) {
  auto task = (CComeTask *)ctx;
  task->readCb();
}

static void sWriteCb(struct bufferevent *bev, void *ctx) {
  auto task = (CComeTask *)ctx;
  task->writeCb();
}

static void sEventCb(struct bufferevent *bev, short what, void *ctx) {
  auto task = (CComeTask *)ctx;
  task->eventCb(what);
}

static void sTimerCb(evutil_socket_t sock, short what, void *ctx) {
  auto task = (CComeTask *)ctx;
  task->timerCb();
}

void CComeTask::setTimer(int ms) {
  if (!getBase()) {
    LOG_ERROR("setTimer failed: base_ not set!");
    return;
  }

  timerEvent_ = event_new(getBase(), -1, EV_PERSIST, sTimerCb, this);
  if (!timerEvent_) {
    LOG_ERROR("set timer failed: event_new failed!");
    return;
  }

  int sec = ms / 1000;          // ��
  int us = (ms % 1000) * 1000;  // ΢��
  timeval tv = { sec, us };
  event_add(timerEvent_, &tv);
}

int CComeTask::readMsg(void *data, int size) {
  if (!bev_) {
    LOG_ERROR("bev not set");
    return 0;
  }
  int res = bufferevent_read(bev_, data, size);
  return res;
}

void CComeTask::beginWrite() {
  if (!bev_) {
    return;
  }
  // ����д��ص�����
  bufferevent_trigger(bev_, EV_WRITE, 0);
}

void CComeTask::eventCb(short what) {
  std::cout << "eventCb " << what << '\n';
  if (what & BEV_EVENT_CONNECTED) {
    std::cout << "BEV_EVENT_CONNECTED\n";
    std::stringstream ss;
    ss << "connect server " << serverIp_ << ':' << serverPort_ << " success!";
    LOG_INFO(ss.str().c_str());
    // ֪ͨ���ӳɹ�
    isConnected_ = true;
    isConnecting_ = false;
    connetedCb();
  }

  // �˳�ʱҪ������
  if (what & BEV_EVENT_ERROR || what & BEV_EVENT_TIMEOUT) {
    std::cout << "BEV_EVENT_ERROR || BEV_EVENT_TIMEOUT\n";
    closeBev();
  }

  if (what & BEV_EVENT_EOF) {
    std::cout << "BEV_EVENT_EOF\n";
    closeBev();
  }
}

//bool CComeTask::writeMsg(const CMsg *msg) {
//  // �����Ϣ�Ϸ���
//  if (!bev_ || !msg || !msg->data_ || msg->size_ <= 0) {
//    std::cerr << "!bev_ || !msg || !msg->data_ || msg->size_ <= 0\n";
//    return false;
//  }
//
//  // д����Ϣͷ
//  int res = bufferevent_write(bev_, msg, sizeof(CMsgHd));
//  if (res != 0) { 
//    std::cerr << "bufferevent_write [CMsgHd] error!\n";
//    return false;  
//  }
//  // д����Ϣ����
//  res = bufferevent_write(bev_, msg->data_, msg->size_);
//  if (res != 0) {
//    std::cerr << "bufferevent_write [msg->data_] error!\n";
//    return false;
//  }
//  return true;
//}

bool CComeTask::writeMsg(const void *data, int size) {
  std::lock_guard<std::mutex> guard(*mtx_);
  if (!bev_ || !data || size <= 0) {
    return false;
  }
  int res = bufferevent_write(bev_, data, size);
  if (res != 0) {
    std::cerr << "CComeTask::writeMsg(const void *data, int size) error!\n";
    return false;
  }
  return true;
}

bool CComeTask::connect() {
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(serverPort_);
  evutil_inet_pton(AF_INET, serverIp_, &addr.sin_addr.s_addr);

  std::lock_guard<std::mutex> guard(*mtx_);
  isConnected_ = false;
  isConnecting_ = false;
  if (!bev_) {
    initBev(-1);
  }
  if (!bev_) {
    LOG_ERROR("CComTask::Connect failed! bev is null!");
    return false;
  }

  // ���� socket ����
  int res = bufferevent_socket_connect(bev_, (sockaddr *)&addr, sizeof(addr));
  if (res != 0) {
    return false;
  }

  isConnecting_ = true;
  return true;
}

bool CComeTask::initBev(int sock) {
  // �� bufferevent �������ӣ�comeSock = -1 ʱ�Զ����� socket
  // ���� bufferevent ������
  bev_ = bufferevent_socket_new(getBase(), sock, BEV_OPT_CLOSE_ON_FREE);
  if (!bev_) {
    LOG_ERROR("bufferevent_socket_new() failed!");
    return false;
  }

  // ���ûص�����
  bufferevent_setcb(bev_, sReadCb, sWriteCb, sEventCb, this);
  // ����Ȩ��
  bufferevent_enable(bev_, EV_READ | EV_WRITE); 
  // ���ó�ʱʱ��
  //timeval tv = { 10, 0 };
  //bufferevent_set_timeouts(bev_, &tv, &tv);
  return true;
}

bool CComeTask::init() {
  // comeSock ���������Ƿ���˻��ǿͻ���
  int comeSock = getSock();
  if (comeSock <= 0) {
    comeSock = -1;
  }

  {
    std::lock_guard<std::mutex> guard(*mtx_);
    initBev(comeSock);
  }

  // ���ӷ�����
  if (serverIp_[0] == '\0') {
    return true;
  }

  return connect();
}

void CComeTask::closeBev() {
  {
    std::lock_guard<std::mutex> guard(*mtx_);
    isConnected_ = false;
    isConnecting_ = false;
    if (bev_) {
      bufferevent_free(bev_);
    }
    bev_ = nullptr;
    if (msg_.data_) {
      delete msg_.data_;
    }
    memset(&msg_, 0, sizeof(msg_));
  }

  // �������Ӷ���ռ䣬����Ͽ���������Ҫ��������
  if (autoDelete_) {
    delete this;
  }
}

bool CComeTask::waitforConnected(int timeoutSec) {
  // ÿ 10 �������һ��
  int milsec = timeoutSec * 100;
  for (int i = 0; i < milsec; ++i) {
    if (isConnected()) {
      return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return isConnected();
}

bool CComeTask::autoConnect(int timeoutSec) {
  // ����������ӣ���ȴ�������ʼ����
  if (isConnected()) {
    return true;
  }
  if (!isConnecting()) {
    connect();
  }
  return waitforConnected(timeoutSec);
}

void CComeTask::setServerIp(const char *ip) { 
  strncpy(this->serverIp_, ip, sizeof(serverIp_));
}