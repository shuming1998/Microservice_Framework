#pragma once
#include <vector>

class CTask;
class CThread;
class CThreadPool {
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

private:
  CThreadPool() {}

  int threadNums_ = 0;  // �߳�����
  int lastThread_ = -1;  // �ϴ��߳�λ��
  std::vector<CThread *> threads_;
};

