#include "cthreadpool.h"
#include "cthread.h"
#include "ctask.h"
#include <thread>
#include <iostream>

#ifdef _WIN32
//��protobufͷ�ļ����г�ͻ ��protobuf��ͷ�ļ�Ҫ��windows.h֮ǰ
#include <windows.h>
#else
#include <signal.h>
#endif

// �����̵߳�ѭ���Ƴ�Ŷ�ж�
static bool isAllExit = false;
// ���е��̶߳���
static std::vector<CThread *> allThreads;
static std::mutex  allThreadsMtx;

void CThreadPool::exitAllThread() {
  isAllExit = true;
  allThreadsMtx.lock();
  for (auto t : allThreads) {
    t->exit();
  }
  allThreadsMtx.unlock();
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void CThreadPool::wait() {
  while (!isAllExit) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

// �̳߳ص�ʵ�ֲ����⿪��
class CCThreadPool : public CThreadPool {
public:
  void init(int threadNums) {
    this->threadNums_ = threadNums;
    this->lastThread_ = -1;
    for (int i = 0; i < threadNums_; ++i) {
      CThread *thread = new CThread();
      std::cout << "create thread NO." << i << '\n';
      thread->id_ = i + 1;
      // �����߳�
      thread->startThread();
      threads_.push_back(thread);
      allThreadsMtx.lock();
      allThreads.push_back(thread);
      allThreadsMtx.unlock();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

  void dispatch(CTask *task) {
    if (!task) {
      return;
    }
    // ��ѯ��ʽ�ַ��߳�
    int tid = (lastThread_ + 1) % threadNums_;
    lastThread_ = tid;
    CThread *t = threads_[tid];

    // �ύ����
    t->addTask(task);

    // �����߳�
    t->activate();
  }

private:
  int threadNums_ = 0;    // �߳�����
  int lastThread_ = -1;   // �ϴηַ��̵߳�λ��
  std::vector<CThread *> threads_;  // �̶߳���

};

CThreadPool *CThreadPoolFactory::create() {
  //socket���ʼ��
  static std::mutex mtx;
  static bool isInit = false;
  mtx.lock();
  if (!isInit) {
#ifdef _WIN32 
    //��ʼ��socket��
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#else
    //ʹ�öϿ�����socket���ᷢ�����źţ���ɳ����˳�
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
      return 1;
    }
#endif
    isInit = true;
  }
  mtx.unlock();
  return new CCThreadPool();
}



