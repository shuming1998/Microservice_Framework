#include <iostream>
#include "cregisterserver.h"

int main(int argc, char *argv[]) {

  std::cout << "Register server\n";
  CRegisterServer server;
  // ��ʼ�������ݶ˿ںŵȲ���
  server.mainFunc(argc, argv);
  // ���������̣߳���ʼ�����˿�
  server.start();
  // �����ȴ��̳߳��˳�
  server.wait();

  return 0;
}