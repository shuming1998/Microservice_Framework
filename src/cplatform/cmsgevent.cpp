#include "cmsgevent.h"
#include "clogclient.h"
#include "cmsgcom.pb.h"
#include "ctools.h"
#include <string>
#include <iostream>
#include <sstream>
#include <map>

// ͬһ������ֻ����һ���ص�����
static std::map<cmsg::CMsgType, CMsgEvent::msgCbFunc> msgCb;

void CMsgEvent::regCb(cmsg::CMsgType type, msgCbFunc func) {
  if (msgCb.find(type) != msgCb.end()) {
    std::stringstream ss;
    ss << "regCb error: " << type << " has set cb";
    LOG_ERROR(ss.str().c_str());
    return;
  }

  msgCb[type] = func;
}

void CMsgEvent::close() {
  clear();
  CComeTask::closeBev();
}

void CMsgEvent::clear() {
  head_.clear();
  msg_.clear();
}

CMsg *CMsgEvent::getMsg() {
  if (msg_.recved()) {
    return &msg_;
  }
  return nullptr;
}

void CMsgEvent::readCb(cmsg::CMsgHead *head, CMsg *msg) {
  // �յ���Ϣ��������Ϣ���͵�����Ϣ�Ļص�����
  auto funcPt = msgCb.find(head->msg_type());
  if (funcPt == msgCb.end()) {
    clear();
    LOG_DEBUG("msg cbfunc not set!");
    return;
  }

  auto func = funcPt->second;
  (this->*func)(head, msg);
}

// ѭ�����ն�����Ϣ
void CMsgEvent::readCb() {
  // TODO������߳��˳��Ƿ�����?
  while (1) {
    if (!recvMsg()) {
      clear();
      return;
    }
    if (!pbHead_) {
      return;
    }
    auto msg = getMsg();
    if (!msg) {
      return;
    }
    // ����������ע��Ļص����������� router �Ǳ�ֻ��Ҫע�ᣬ����Ҫ����

    // ������ѭ��
    if (pbHead_->msg_type() != cmsg::MSG_ADD_LOG_REQ) {
      std::string ss;
      ss = "CMsgEvent::readCb(): ";
      ss += pbHead_->service_name();
      LOG_DEBUG(ss.c_str());
    }

    readCb(pbHead_, msg);
    clear();
  }
}

bool CMsgEvent::recvMsg() {
  // ���
  // 1 ��Ϣͷ��С
  if (!head_.size_) {
    int len = readMsg(&head_.size_, sizeof(head_.size_));
    if (len <= 0 || head_.size_ <= 0) {
      return false;
    }
    if (!head_.alloc(head_.size_)) {
      std::cerr << "head_.alloc failed!\n";
      return false;
    }
  }

  // 2 ��ʼ������Ϣͷ(��Ȩ����Ϣ��С)
  if (!head_.recved()) {
    // �ڶ��ν�������ϴε�λ�ÿ�ʼ��
    int len = readMsg(head_.data_ + head_.recvSize_, head_.size_ - head_.recvSize_);
    if (len <= 0) {
      return true;
    }
    head_.recvSize_ += len;
    // ������ݻ���û�н�����ɣ�������һ�ν���
    if (!head_.recved()) {
      return true;
    }
    // ������ͷ�����ݽ������
    // �����л�
    if (!pbHead_) {
      pbHead_ = new cmsg::CMsgHead();
    }
    if (!pbHead_->ParseFromArray(head_.data_, head_.size_)) {
      std::cerr << "pbHead.ParseFromArray failed!\n";
      return false;
    }

    // ����հ�����
    if (pbHead_->msg_size() == 0) {
      LOG_DEBUG("recv msg which size = 0!");
      msg_.type_ = pbHead_->msg_type();
      msg_.size_ = 0;
      return true;
    } else {  // ����������Ϣ
      // ��Ȩ
      // ��ȡ��Ϣ���ݴ�С����ռ�
      bool is = msg_.alloc(pbHead_->msg_size());
      if (!is) {
        std::cerr << "msg_.alloc failed!\n";
        return false;
      }
    }
    // ������Ϣ����
    msg_.type_ = pbHead_->msg_type();
  }

  // 3 ��ʼ������Ϣ����
  if (!msg_.recved()) {
    int len = readMsg(msg_.data_ + msg_.recvSize_, msg_.size_ - msg_.recvSize_);
    if (len <= 0) {
      return true;
    }
    msg_.recvSize_ += len;
  }
  if (msg_.recved()) {
    std::cout <<"msg_ recved()" << '\n';
  }
  return true;
}

bool CMsgEvent::sendMsg(cmsg::CMsgHead *head, CMsg *msg) {
  if (!head || !msg) {
    return false;
  }
  head->set_msg_size(msg->size_);
  // ��Ϣͷ���л�
  std::string headStr = head->SerializeAsString();
  int headSize = headStr.size();

  // 1 ������Ϣͷ��С �ݲ������ֽ�������
  int res = writeMsg(&headSize, sizeof(headSize));
  if (!res) {
    return false;
  }

  // 2 ������Ϣͷ(pb ���л�) CMsgHead(����Ҫ���ü������͵���Ϣ���ݴ�С)
  res = writeMsg(headStr.data(), headStr.size());
  if (!res) {
    return false;
  }

  res = writeMsg(msg->data_, msg->size_);
  if (!res) {
    return false;
  }

  return true;
}

bool CMsgEvent::sendMsg(cmsg::CMsgHead *head, const google::protobuf::Message *message) {
  if (!message || !head) {
    return false;
  }

  // ���
  // ��Ϣ�������л�
  std::string msgStr = message->SerializeAsString();
  int msgSize = msgStr.size();
  CMsg msg;
  msg.data_ = (char *)msgStr.data();
  msg.size_ = msgSize;
  return sendMsg(head, &msg);

  //head->set_msg_size(msgSize);

  //// ��Ϣͷ���л�
  //std::string headStr = head->SerializeAsString();
  //int headSize = headStr.size();

  //// 1 ������Ϣͷ��С �ݲ������ֽ�������
  //int res = writeMsg(&headSize, sizeof(headSize));
  //if (!res) {
  //  return false;
  //}

  //// 2 ������Ϣͷ(pb ���л�) CMsgHead(����Ҫ���ü������͵���Ϣ���ݴ�С)
  //res = writeMsg(headStr.data(), headStr.size());
  //if (!res) {
  //  return false;
  //}

  //// 3 ������Ϣ����(pb ���л�) ҵ�� proto
  //res = writeMsg(msgStr.data(), msgStr.size());
  //if (!res) {
  //  return false;
  //}

  //return true;
}

bool CMsgEvent::sendMsg(cmsg::CMsgType type, const google::protobuf::Message *message) {
  if (!message) {
    return false;
  }
  cmsg::CMsgHead head;
  head.set_msg_type(type);

  return sendMsg(&head, message);
}