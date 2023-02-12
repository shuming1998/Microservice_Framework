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
	// ����ģʽ��ȡ�̳߳ض���
	static CThreadPool* get() {
		static CThreadPool p;
		return &p;
	}

  // ��ʼ�������̲߳�����
  void init(int threadNums);

  // �ַ��߳�
  void dispatch(CTask *task);

  CThreadPool() {}
private:

  int threadNums_ = 0;    // �߳�����
  int lastThread_ = -1;   // �ϴηַ��̵߳�λ��
  std::vector<CThread *> threads_;  // �̶߳���
};


#endif

