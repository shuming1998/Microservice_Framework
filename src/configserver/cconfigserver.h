#ifndef CCONFIG_SERVER_H
#define CCONFIG_SERVER_H
#include "cservice.h"



class CConfigServer : public CService {
public:
  virtual CServiceHandle *createServiceHandle() override;

  // ���ݲ�����ʼ��������Ҫ�ȵ���
  void mainFunc(int argc, char *argv[]);

  // �ȴ��߳��˳�
  void wait();

private:


};



#endif

