#include "cmsgevent.h"
#include "cmsgcom.pb.h"
#include <event2/bufferevent.h>
#include <string>
#include <iostream>

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

bool CMsgEvent::recvMsg() {
  if (!bev_) {
    std::cerr << "CMsgEvent::recvMsg() failed��bev not set" << '\n';
    return false;
  }
  // ���
  // 1 ��Ϣͷ��С
  if (!head_.size_) {
    int len = bufferevent_read(bev_, &head_.size_, sizeof(head_.size_));
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
    int len = bufferevent_read(bev_, head_.data_ + head_.recvSize_, head_.size_ - head_.recvSize_);
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
    cmsg::CMsgHead pbHead;
    if (!pbHead.ParseFromArray(head_.data_, head_.size_)) {
      std::cerr << "pbHead.ParseFromArray failed!\n";
      return false;
    }

    // ��Ȩ
    // ��ȡ��Ϣ���ݴ�С����ռ�
    if (!msg_.alloc(pbHead.msg_size())) {
      std::cerr << "msg_.alloc failed!\n";
      return false;
    }
    // ������Ϣ����
    msg_.type_ = pbHead.msg_type();
  }
  
  // 3 ��ʼ������Ϣ����
  if (!msg_.recved()) {
    int len = bufferevent_read(bev_, msg_.data_ + msg_.recvSize_, msg_.size_ - msg_.recvSize_);
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

void CMsgEvent::sendMsg(cmsg::CMsgType type, const google::protobuf::Message *message) {
  if (!bev_ || !message) {
    return;
  }

  // ��Ϣ����
  std::string msgStr = message->SerializeAsString();
  int msgSize = msgStr.size();
  cmsg::CMsgHead head;
  head.set_msg_type(type);
  head.set_msg_size(msgSize);

  // ��Ϣͷ
  std::string headStr = head.SerializeAsString();
  int headSize = headStr.size();

  // 1 ������Ϣͷ��С �ݲ������ֽ�������
  bufferevent_write(bev_, &headSize, sizeof(headSize));

  // 2 ������Ϣͷ CMsgHead(����Ҫ���ü������͵���Ϣ���ݴ�С)
  bufferevent_write(bev_, headStr.data(), headStr.size());

  // 3 ������Ϣ���� 
  bufferevent_write(bev_, msgStr.data(), msgStr.size());
}