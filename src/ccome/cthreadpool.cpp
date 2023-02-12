#include "cthreadpool.h"
#include "cthread.h"
#include "ctask.h"
#include <thread>
#include <iostream>

void CThreadPool::init(int threadNums) {
  this->threadNums_ = threadNums;
  this->lastThread_ = -1;
  for (int i = 0; i < threadNums_; ++i) {
    CThread *thread = new CThread();
    std::cout << "create thread NO." << i << '\n';
    thread->id_ = i + 1;
    // �����߳�
    thread->startThread();
    threads_.push_back(thread);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void CThreadPool::dispatch(CTask *task) {
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