#ifndef CAUTH_DAO_H
#define CAUTH_DAO_H

namespace cmsg {
  class CAddUserReq;
  class CLoginReq;
  class CLoginRes;
}

class CAuthDAO {
public:
  ~CAuthDAO() {}
  static CAuthDAO *get() {
    static CAuthDAO dao;
    return &dao;
  }

  // ��ʼ�����ݿ⣬׼���û�������
  bool init();

  // ��װ��
  bool install();

  // ����û�
  bool addUser(cmsg::CAddUserReq *user);

  // ��¼���ݿ�
  // @param userReq �û���¼��Ϣ����ʱ�����Ѽ���
  // @param userRes �����û� token
  // @param timeoutSec token ��ʱʱ��
  bool login(const cmsg::CLoginReq *userReq, cmsg::CLoginRes *userRes, int timeoutSec);

private:
  CAuthDAO() {}

};







#endif // ! CAUTH_DAO_H

