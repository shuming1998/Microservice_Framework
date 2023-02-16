#ifndef CAUTH_CLIENT_H
#define CAUTH_CLIENT_H
#include "cserviceclient.h"
#include <string>
#include <map>
#include <mutex>
#include <thread>

class CAuthClient : public CServiceClient {
public:
  static CAuthClient *get() {
    static CAuthClient client;
    return &client;
  }

  CAuthClient() {}
  ~CAuthClient() {}

  // ������¼����
  // @param username �û���
  // @param password �������룬�ں����лᾭ�� md5 base64 �������
  void LoginReq(std::string username, std::string password);

  // ����û�����
  void addUserReq(cmsg::CAddUserReq *user);

  // ע������Ļص�����
  static void regMsgCallback() {
    regCb(cmsg::MSG_LOGIN_RES, (msgCbFunc)&CAuthClient::LoginRes);
    regCb(cmsg::MSG_ADD_USER_RES, (msgCbFunc)&CAuthClient::addUserRes);
  }

  // ��ȡ������û���¼��Ϣ
  bool getLoginInfo(std::string username, cmsg::CLoginRes *outInfo, int timeoutMs);

private:
  // ������¼�������Ӧ
  void LoginRes(cmsg::CMsgHead *head, CMsg *msg);
  // ����û��������Ӧ
  void addUserRes(cmsg::CMsgHead *head, CMsg *msg);

  std::map<std::string, cmsg::CLoginRes> loginMap_;   // �洢�û��ĵ�¼��Ϣ
  std::mutex loginMapMtx_;
};




#endif

