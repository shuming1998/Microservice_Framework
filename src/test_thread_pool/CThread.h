#pragma once
#include <event2/event.h>
#include <list>
#include <mutex>

class CTask;
class CThread {
public:
  CThread();
  ~CThread();
  // �����߳�
  void startThread();

  // �߳���ں���
  void mainFunc();

  // ��װ�̣߳���ʼ�� event_base �͹ܵ�(linux)/socket(windows)�����¼������ڼ����߳�
  bool setup();

  // �յ����̷߳����ļ�����Ϣ(�̳߳صķַ�)
  void notify(evutil_socket_t fd, short which);

  // �����߳�
  void activate();

  // �����Ҫ���������һ���̶߳�Ӧһ�� event_base��������ͬʱ����������
  void addTask(CTask *task);

  void setId(int id) { id_ = id; }

  int getId() const { return id_; }


private:
  int id_ = 0;                // �̱߳��
  int notifySendFd_ = 0;
  struct event_base *base_ = nullptr;

  std::list<CTask *> tasks_;  // �����б�
  std::mutex taskMutex_;
};

