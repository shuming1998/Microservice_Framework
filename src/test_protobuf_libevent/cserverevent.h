#ifndef CSERVER_EVENT_H
#define CSERVER_EVENT_H


#include "cmsgevent.h"
class CServerEvent : public CMsgEvent {
public:
  typedef void(CServerEvent::*msgCbFunc)(const char *data, int size);

  // ��ʼ���ص�����
  static void init();

  // ���յ�¼������Ϣ
  // @param data ��Ϣ����
  // @param size ��Ϣ��С
  void loginReq(const char *data, int size);

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

#endif // !CSERVER_EVENT_H

