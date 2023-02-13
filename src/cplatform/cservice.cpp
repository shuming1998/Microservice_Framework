#include "cservice.h"
#include "ctools.h"
#include "cservicehandle.h"
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <string.h>
#include <sstream>

static void sListenCb(struct evconnlistener *ev, evutil_socket_t sock, struct sockaddr *addr, int socklen, void *arg) {
  LOG_DEBUG("sListenCb");
  auto task = (CService *)arg;
  task->listenCb(sock, addr, socklen);
}

CService::CService() {
  threadPoolForListen_ = CThreadPoolFactory::create();
  threadPoolForClient_ = CThreadPoolFactory::create();
}

CService::~CService() {
  delete threadPoolForListen_;
  threadPoolForListen_ = nullptr;
  delete threadPoolForClient_;
  threadPoolForClient_ = nullptr;
} 


void CService::listenCb(int clientSock, struct sockaddr *clientAddr, int socklen) {
  // �����ͻ��˴������
  auto handle = createServiceHandle();
  handle->setSock(clientSock);
  // ���� ssl ������
  handle->setSslCtx(this->sslCtx());

  std::stringstream ss;
  char ip[16] = { 0 };
  auto addr = (sockaddr_in *)clientAddr;
  evutil_inet_ntop(AF_INET, &addr->sin_addr.s_addr, ip, sizeof(ip));
  ss << "client ip: " << ip << " port: " << addr->sin_port << '\n';
  LOG_INFO(ss.str().c_str());

  // �����������뵽�̳߳أ����������� servicehandle ���д���
  handle->setClientIp(ip);
  handle->setClientPort(addr->sin_port);
  threadPoolForClient_->dispatch(handle);
}

// �����ʼ�������̳߳ص���
bool CService::init() {
  if (serverPort_ <= 0) {
    LOG_ERROR("serverPort_ <= 0!\n")
    return false;
  }

  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(serverPort_);
  auto evc = evconnlistener_new_bind(getBase(), sListenCb, this, 
                                     LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, 
                                     10, 
                                     (sockaddr *)&addr, sizeof(addr));
  if (!evc) {
    std::stringstream ss;
    ss << "listen port " << serverPort_ << " failed!\n";
    LOG_ERROR(ss.str().c_str());
    return false;
  }

  std::stringstream ss;
  ss << "listen port " << serverPort_ << " success!\n";
  LOG_INFO(ss.str().c_str());
  return true;
}

// ��ʼ�������У������������񣬼��뵽�̳߳�
bool CService::start() {
  threadPoolForListen_->init(1);
  threadPoolForClient_->init(threadCount_);
  threadPoolForListen_->dispatch(this);
  return true;
}