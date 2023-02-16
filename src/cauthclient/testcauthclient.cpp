#include "cauthclient.h"
#include <thread>
#include <iostream>

int main() {
  CAuthClient::regMsgCallback();
  CAuthClient client;
  client.setServerIp("127.0.0.1");
  client.setServerPort(AUTH_PORT);
  client.startConnect();
  while (!client.isConnected()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // ����û�
  cmsg::CAddUserReq addUser;
  addUser.set_username("root");
  addUser.set_password("123456");
  addUser.set_rolename("root");
  client.addUserReq(&addUser);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // �����¼����
  client.LoginReq("root", "123456");

  // �洢��¼�� token ��Ϣ

  CThreadPool::wait();
  return 0;
}

