#ifndef CCONFIG_SERVER_H
#define CCONFIG_SERVER_H
#include "cservice.h"



class CConfigServer : public CService {
public:
  CServiceHandle *createServiceHandle();

  // ���ݲ�����ʼ��������Ҫ�ȵ���
  void mainFunc(int argc, char *argv[]);

  // �ȴ��߳��˳�
  void wait();
};



#endif

