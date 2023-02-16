#include "cserviceproxyclient.h"
#include "clogclient.h"
#include "ctools.h"

bool CServiceProxyClient::sendMsg(cmsg::CMsgHead *head, CMsg *msg, CMsgEvent *ev) {
  regEvent(ev);
  head->set_msg_id((long long)ev);
  return CMsgEvent::sendMsg(head, msg);
}

void CServiceProxyClient::regEvent(CMsgEvent *ev) {
  std::lock_guard<std::mutex> guard(callbackTasksMtx_);
  callbackTasks_[(long long)ev] = ev;
}

void CServiceProxyClient::delEvent(CMsgEvent *ev) {
  std::lock_guard<std::mutex> guard(callbackTasksMtx_);
  callbackTasks_.erase((long long)ev);
}

void CServiceProxyClient::readCb(cmsg::CMsgHead *head, CMsg *msg) {
  if (!head || !msg) {
    return;
  }
  std::lock_guard<std::mutex> guard(callbackTasksMtx_);
  // ת���� RouterHandle
  // ÿ�� CServiceProxyClient ������ܹ������ CRouterHandle (CMsgEvent)
  auto router = callbackTasks_.find(head->msg_id());
  if (router == callbackTasks_.end()) {
    LOG_DEBUG("callbackTasks_ can't find router!");
    return;
  }
  // ���߳����⣿
  router->second->sendMsg(head, msg);
}