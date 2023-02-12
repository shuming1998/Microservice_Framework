#ifndef  CMSG_CLIENT_H
#define  CMSG_CLIENT_H
#include <string>

class CMsgClient {
public:
  // ��ʱ���������̣߳��������ݸ������
  void startThread();

  // �߳�������
  void mainFunc();

  void setServerPort(int port) { this->serverPort_ = port; }
  void setServerIp(std::string ip) { this->serverIp_ = ip; }

private:
  int serverPort_ = 0;
  std::string serverIp_ = "";

};


#endif // ! CMSG_CLIENT_H

