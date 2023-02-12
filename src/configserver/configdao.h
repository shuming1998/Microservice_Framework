#ifndef CONFIG_DAO_H
#define CONFIG_DAO_H
#include "cmsgcom.pb.h"

namespace cmysql {
  class CMysql;
}

class ConfigDAO {
public:
  virtual ~ConfigDAO() {}

  static ConfigDAO *get() {
    static ConfigDAO dao;
    return &dao;
  }

  // ��ʼ�����ݿ�
  bool init(const char *ip, const char *user, const char *password, const char *db, int port = 3306);

  // ��װ���ݿ�ı�
  bool install();

  // �������ã�������о͸���
  bool uploadConfig(cmsg::CConfig *conf);

  // ��ȡ����
  cmsg::CConfig downloadConfig(const char *ip, int port);

  // ��ȡ��ҳ�������б�
  cmsg::CConfigList downloadAllConfig(int page, int pageCount);

  // ɾ��ָ��΢��������
  bool deleteConfig(const char *ip, int port);

private:
  ConfigDAO() {}

  // mysql ���ݿ�������
  cmysql::CMysql *mysql_ = nullptr;

};

#endif

