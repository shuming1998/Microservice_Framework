#ifndef CMYSQL_H
#define CMYSQL_H

#include "cdata.h"
#include <vector>

struct MYSQL;
struct MYSQL_RES;
namespace cmysql {

class CMYSQL_API CMysql {
public:
  // ��ʼ�� MYSQL API
  bool init();

  // ����ռ�õ�������Դ
  void close();

  // �������ݿ�����(�������̰߳�ȫ)
  // @param flag ����֧�ֶ������
  bool connect(const char *host, const char *user, const char *password, const char *db, unsigned short port = 3306, /*const char *unixSock = 0,*/ unsigned long flag = 0);

  // ִ�� sql ���. ��� sqlLen = 0������ strlen ��ȡ�ַ�������
  bool query(const char *sql, unsigned long sqlLen = 0);

  // Mysql �����趨(��ʱʱ�䡢�Զ�������������֮ǰ����)
  bool option(COption opt, const void *arg);

  // �������ӳ�ʱʱ��
  bool setConnectTimeout(int sec);

  // ���ó�ʱ����
  bool setReconnect(bool is = true);

  // ��ȡ�����
  bool storeResult();   // ����ȫ�����
  bool useResult();     // ��ʼ���ս����ͨ�� fetch ��ȡ���

  // �ͷŽ����ռ�õĿռ�
  void freeResult();

  // ��ȡһ������
  std::vector<CData> fetchRow();

  // ���� insert sql ���
  std::string getInsertSql(MData kv, std::string table);

  // ����Ƕ���������
  bool insert(MData kv, std::string table);

  // �������������
  bool insertBin(MData kv, std::string table);

  // ��ȡ update ���ݵ� sql ��䣬�û�Ҫ���� where
  std::string getUpdateSql(MData kv, std::string table, std::string where);

  // ִ�� update�����ظ���������ʧ�ܷ��� -1
  int update(MData kv, std::string table, std::string where);

  // update ����������
  int updateBin(MData kv, std::string table, std::string where);

  // ����ӿ�
  bool startTransaction();
  bool stopTransaction();
  bool commit();
  bool rollback();

  // ���׽ӿڣ����� select �����ݽ����ÿ�ε���ʱ�����ϴεĽ����
  CRows getResult(const char *sql);

protected:
  MYSQL *mysql_ = nullptr;
  MYSQL_RES *result_ = nullptr;

};

}// namespace cmysql

#endif
