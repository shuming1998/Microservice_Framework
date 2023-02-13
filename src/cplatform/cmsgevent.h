#ifndef CMSG_EVENT_H
#define CMSG_EVENT_H
#include "cmsg.h"
#include "ccometask.h"
#include "cmsgtype.pb.h"
#include "cmsgcom.pb.h"

// ���ٵ��� bufferevent �ӿڡ�ֱ�ӵ��� CComeTask ��װ
class CCOME_API CMsgEvent : public CComeTask {
public:
  // ������Ϣ���ַ���Ϣ
  virtual void readCb();
  // ��Ϣ�ص�������Ĭ�Ϸ��͵�����ע��ĺ�������·������
  virtual void readCb(cmsg::CMsgHead *head, CMsg *msg);

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
  // @return false: bevδ���õ�ԭ��
  virtual bool sendMsg(cmsg::CMsgType type, const google::protobuf::Message *message);
  virtual bool sendMsg(cmsg::CMsgHead *head, const google::protobuf::Message *message);
  virtual bool sendMsg(cmsg::CMsgHead *head, CMsg *msg);

  // �����棬���ڽ�����һ����Ϣ
  void clear();

  void close();

  typedef void(CMsgEvent::*msgCbFunc)(cmsg::CMsgHead *head, CMsg *msg);

  // �����Ϣ����Ļص�������������Ϣ���ͷַ�   ͬһ������ֻ����һ���ص�����
  static void regCb(cmsg::CMsgType type, msgCbFunc func);

private:
  CMsg head_;   //��Ϣͷ
  CMsg msg_;    // ��Ϣ����

  cmsg::CMsgHead *pbHead_ = nullptr; // pb ��Ϣͷ
};

#endif // !CMSG_EVENT_H

