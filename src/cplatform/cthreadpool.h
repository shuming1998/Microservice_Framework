#ifndef CTHREAD_POOL_H
#define CTHREAD_POOL_H

#ifdef _WIN32
#ifdef CCOME_EXPORTS
#define CCOME_API __declspec(dllexport)
#else
#define CCOME_API __declspec(dllimport)
#endif
#else
#define CCOME_API
#endif // _WIN32


#include <vector>

class CTask;
class CThread;
class CCOME_API CThreadPool {
public:
  // ��ʼ�������̲߳�����
  virtual void init(int threadNums) = 0;

  // �ַ��߳�
  virtual void dispatch(CTask *task) = 0;

  // �˳������߳�
  static void exitAllThread();

  // �����ȴ� exitAllThread
  static void wait();
};

class CCOME_API CThreadPoolFactory {
public:
  // �����̳߳ض���
  static CThreadPool *create();
};

#endif

