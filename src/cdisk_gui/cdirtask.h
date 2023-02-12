#ifndef CDIR_TASK_H
#define CDIR_TASK_H

#include "ccometask.h"
#include <functional>

typedef void(*dirCbFunc)(std::string dir);

class CDirTask : public CComeTask {
public:
  // ���յ���Ϣ������Ϣ�Ļص�
  bool readCb(const CMsg *msg) override;
  // ���ӳɹ�����Ϣ�ص�
  void connetedCb() override;

  dirCbFunc dirCb_ = nullptr;  // ��ȡĿ¼����õĻص�����

  void setServerRoot(std::string root) { this->serverRoot_ = root; }

private:
  std::string serverRoot_ = "";
};


#endif // !CDIR_TASK_H

