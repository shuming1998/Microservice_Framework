#ifndef CREGISTER_SERVER_H
#define CREGISTER_SERVER_H
#include "cservice.h"

// ע�����ķ����
class CRegisterServer : public CService {
public:
  virtual CServiceHandle *createServiceHandle() override;

  // ���ݲ�����ʼ��������Ҫ�ȵ���
  void mainFunc(int argc, char *argv[]);

  // �ȴ��߳��˳�
  void wait();


private:

};


#endif

