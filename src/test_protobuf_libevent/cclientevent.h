#ifndef CCLIENT_EVENT_H
#define CCLIENT_EVENT_H

#include "cmsgevent.h"
#include "cmsgtype.pb.h"
#include <map>

class CClientEvent : public CMsgEvent {
public:
  typedef void(CClientEvent::*msgCbFunc)(const char *data, int size);

  // ��ʼ���ص�����
  static void init();

  // ���յ�¼������Ϣ
  // @param data ��Ϣ����
  // @param size ��Ϣ��С
  void loginRes(const char *data, int size);

  // ע����Ϣ�ص�������ֻ��Ҫע��һ�Σ����ھ�̬ map ��
  // @param type ��Ϣ����
  // @param func ��Ϣ�ص�����
  static void regCb(cmsg::CMsgType type, msgCbFunc func) {
    cbs_[type] = func;
  }

  // ͨ�����ͺͳ�Ա�������ú���
  // @param type ��Ϣ����
  // @param data ��Ϣ����
  // @param size ��Ϣ��С
  void callFunc(cmsg::CMsgType type, const char *data, int size) {
    if (cbs_.find(type) != cbs_.end()) {
      (this->*cbs_[type])(data, size);
    }
  }

private:
  static std::map<cmsg::CMsgType, msgCbFunc> cbs_;
};

#endif // ! CCLIENT_EVENT_H

