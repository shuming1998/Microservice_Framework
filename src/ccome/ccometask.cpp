#include "ccometask.h"
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <iostream>
#include <string.h>

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
    // ֪ͨ���ӳɹ�
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

void CComeTask::readCb() {
  // for ѭ����Ŀ�ģ�ȷ����Ե����ʱ�ܶ�ȡ����������
  for (;;) {
    // �����˲�������Ϣ
    if (!isRecvMsg_) {
      int len = bufferevent_read(bev_, &readBuf_, sizeof(readBuf_));
      if (len <= 0) {
        return;
      }
      readCb(readBuf_, len);
      continue;
    }
    // ������Ϣ CMsgHead
    if (!msg_.data_) {
      int hdSize = sizeof(CMsgHd);
      int len = bufferevent_read(bev_, &msg_, hdSize);
      if (len <= 0) {
        return;
      }
      if (len != hdSize) {
        std::cout << "len is: " << len << '\n';
        std::cerr << "msg head recv error!\n";
        return;
      }

      // ��֤��Ϣͷ�Ƿ���Ч
      if (msg_.type_ >= MSG_MAX_TYPE || msg_.size_ <= 0 || msg_.size_ > MSG_MAX_SIZE) {
        std::cerr << "msg head is error!\n";
        return;
      }
      msg_.data_ = new char[msg_.size_];
    }

    int readSize = msg_.size_ - msg_.recved_;
    if (readSize <= 0) {
      delete msg_.data_;
      memset(&msg_, 0, sizeof(msg_));
      return;
    }

    int len = bufferevent_read(bev_, msg_.data_ + msg_.recved_, readSize);
    if (len <= 0) {
      return;
    }
    msg_.recved_ += len;

    if (msg_.recved_ == msg_.size_) {
      // ������Ϣ
      std::cout << "recved msg " << msg_.size_ << '\n';
      // ��������������� bev_ ��������
      if (!readCb(&msg_)) {
        return;
      }
      // ��Ϣ������ϣ��ͷſռ�
      delete msg_.data_;
      memset(&msg_, 0, sizeof(msg_));
    }
  }
}

bool CComeTask::writeMsg(const CMsg *msg) {
  // �����Ϣ�Ϸ���
  if (!bev_ || !msg || !msg->data_ || msg->size_ <= 0) {
    std::cerr << "!bev_ || !msg || !msg->data_ || msg->size_ <= 0\n";
    return false;
  }

  // д����Ϣͷ
  int res = bufferevent_write(bev_, msg, sizeof(CMsgHd));
  if (res != 0) { 
    std::cerr << "bufferevent_write [CMsgHd] error!\n";
    return false;  
  }
  // д����Ϣ����
  res = bufferevent_write(bev_, msg->data_, msg->size_);
  if (res != 0) {
    std::cerr << "bufferevent_write [msg->data_] error!\n";
    return false;
  }
  return true;
}

bool CComeTask::writeMsg(const void *data, int size) {
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

bool CComeTask::init() {
  // comeSock ���������Ƿ���˻��ǿͻ���
  int comeSock = getSock();
  if (comeSock <= 0) {
    comeSock = -1;
  }
  // �� bufferevent �������ӣ�comeSock = -1 ʱ�Զ����� socket
  // ���� bufferevent ������
  bev_ = bufferevent_socket_new(getBase(), comeSock, BEV_OPT_CLOSE_ON_FREE);
  if (!bev_) {
    std::cerr << "bufferevent_socket_new() failed!\n";
    return false;
  }

  // ���ûص�����
  bufferevent_setcb(bev_, sReadCb, sWriteCb, sEventCb, this);
  // ����Ȩ��
  bufferevent_enable(bev_, EV_READ | EV_WRITE);
  // ���ó�ʱʱ��
  timeval tv = { 10, 0 };
  bufferevent_set_timeouts(bev_, &tv, &tv);

  // ���ӷ�����
  if (serverIp_.empty()) {
    return true;
  }
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(serverPort_);
  evutil_inet_pton(AF_INET, serverIp_.c_str(), &addr.sin_addr.s_addr);

  int res = bufferevent_socket_connect(bev_, (sockaddr *)&addr, sizeof(addr));
  if (res != 0) {
    return false;
  }

  return true;
}

void CComeTask::closeBev() {
  if (bev_) {
    bufferevent_free(bev_);
  }
  bev_ = nullptr;
  if (msg_.data_) {
    delete msg_.data_;
  }
  memset(&msg_, 0, sizeof(msg_));
  delete this;
}