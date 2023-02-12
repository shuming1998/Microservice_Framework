#ifndef CMSG_EVENT_H
#define CMSG_EVENT_H
#include "cmsg.h"
#include "cmsgtype.pb.h"

class CMsgEvent {
public:

  // �������ݰ�
  // 1. ��ȷ������Ϣ��������Ϣ������
  // 2. ��Ϣ���ղ��������ȴ���һ��
  // 3. ��Ϣ���ճ����˳�������ռ�
  // return true: 1/2  false: 3 
  bool recvMsg();

  CMsg *getMsg();

  // ������Ϣ������ͷ��(�Զ�����)
  // @param type ��Ϣ����
  // @param message ��Ϣ����
  void sendMsg(cmsg::CMsgType type, const google::protobuf::Message *message);

  void setBev(struct bufferevent *bev) { this->bev_ = bev; }

  // �����棬���ڽ�����һ����Ϣ
  void clear();

private:
  struct bufferevent *bev_ = nullptr;
  CMsg head_;   //��Ϣͷ
  CMsg msg_;    // ��Ϣ����

};

#endif // !CMSG_EVENT_H

