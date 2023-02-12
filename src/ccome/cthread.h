#ifndef CTHREAD_H
#define CTHREAD_H

#include <list>
#include <mutex>

class CTask;
class CThread {
public:
  // �����߳�
  void startThread();

  // �߳���ں���
  void mainFunc();

  // ��װ�̣߳���ʼ�� event_base �͹ܵ�(linux)/socket(windows)�����¼������ڼ����߳�
  bool setup();

  // �յ����̷߳����ļ�����Ϣ(�̳߳صķַ�)
  void notify(int fd, short which);

  // �����߳�
  void activate();

  // �����Ҫ���������һ���̶߳�Ӧһ�� event_base��������ͬʱ����������
  void addTask(CTask *task);

  int id_ = 0;                // �̱߳��
private:
  int notifySendFd_ = 0;
  struct event_base *base_ = nullptr;

  std::list<CTask *> tasks_;  // �����б�
  std::mutex taskMutex_;
};

#endif // !CTHREAD_H