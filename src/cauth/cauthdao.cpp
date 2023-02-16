#include "cauthdao.h"
#include "cmysql.h"
#include "ctools.h"
#include "cmsgcom.pb.h"
#include <thread>
#include <string>
#include <mutex>
#include <time.h>

static cmysql::CMysql mysql;
static std::mutex authMtx;

bool CAuthDAO::init() {
  std::lock_guard<std::mutex> guard(authMtx);
  if (!mysql.init()) {
    LOG_DEBUG("mysql init failed!");
    return false;
  }
  mysql.setReconnect(true);
  mysql.setConnectTimeout(3);

  if (!mysql.inputDBConfig()) {
    LOG_DEBUG("mysql.inputDBConfig() failed!");
    return false;
  }


  return true;
}


bool CAuthDAO::install() {
  // �����û���Ȩ��
  std::string sql = "\
                    CREATE TABLE IF NOT EXISTS `cms_auth` (\
                    `id` INT AUTO_INCREMENT, \
                    `cms_username` VARCHAR(128), \
                    `cms_password` VARCHAR(1024), \
                    `cms_rolename` VARCHAR(128), \
                    PRIMARY KEY(`id`), \
                      UNIQUE KEY `cms_username_UNIQUE` (`cms_username`));";
  std::lock_guard<std::mutex> guard(authMtx);
  if (!mysql.query(sql.c_str())) {
    LOG_DEBUG("create table cms_auth failed!");
    return false;
  }
  // �����û� token ��
  sql = "\
        CREATE TABLE IF NOT EXISTS `cms_token` (\
        `id` INT AUTO_INCREMENT, \
        `cms_username` VARCHAR(1024), \
        `cms_rolename` VARCHAR(128), \
        `token` VARCHAR(64), \
        `expired_time` int, \
        PRIMARY KEY(`id`));";
  if (!mysql.query(sql.c_str())) {
    LOG_DEBUG("create table cms_token failed!");
    return false;
  }

  return true;
}

bool CAuthDAO::addUser(cmsg::CAddUserReq *user) {
  cmysql::MData data;
  data["cms_username"] = user->username().c_str();
  data["cms_password"] = user->password().c_str();  // �����Ѽ���
  data["cms_rolename"] = user->rolename().c_str();

  std::lock_guard<std::mutex> guard(authMtx);
  if (!mysql.insert(data, "cms_auth")) {
    LOG_DEBUG("mysql.insert failed!");
    return false;
  }

  return true;
}

bool CAuthDAO::login(const cmsg::CLoginReq *userReq, cmsg::CLoginRes *userRes, int timeoutSec) {
  // 1 ��֤�û�������
  std::stringstream ss;
  ss << "select cms_username,cms_rolename from cms_auth ";
  // ����ע�빥��
  std::string username = userReq->username();
  if (username.find(';') != username.npos || username.find('\'') != username.npos) {
    return false;
  }
  ss << " where cms_username='" << userReq->username() << "'";
  ss << " and cms_password='" << userReq->password() << "'";
  LOG_DEBUG(ss.str().c_str());
  auto rows = mysql.getResult(ss.str().c_str());
  if (rows.size() == 0) {
    userRes->set_res(cmsg::CLoginRes::ERROR);
    userRes->set_token("username or password error!");
    return false;
  }

  // �ӵ�һ������еĵڶ����ֶ�ȡ�� rolename
  std::string rolename = "";
  if (rows[0][1].data) {
    rolename = rows[0][1].data;
  }
 
  userRes->set_username(username);
  userRes->set_rolename(rolename);

  // 2 ���� token��ʹ�����ݿ� uuid �ӿ�
  cmysql::MData data;
  data["@token"] = "UUID()";
  data["cms_username"] = username.c_str();
  data["cms_rolename"] = rolename.c_str();
  // ��ʱʱ���õ�ǰʱ����ӳ�ʱʱ��
  int now = time(0);
  int expiredTime = now + timeoutSec;
  ss.str("");
  ss << expiredTime;
  std::string sExpiredTime = ss.str();
  data["expired_time"] = sExpiredTime.c_str();
  // ��������
  if (!mysql.insert(data, "cms_token")) {
    userRes->set_res(cmsg::CLoginRes::ERROR);
    userRes->set_token("1 make token error!");
    return false;
  }

  // ���ݸղŲ�������� id �ţ������ݿ���ȡ���������ݣ��Ӷ���ȡ token
  int id = mysql.getInsertId();
  ss.str("");
  ss << "select token from cms_token where id=" << id;
  rows = mysql.getResult(ss.str().c_str());
  if (rows.size() == 0) { 
    userRes->set_res(cmsg::CLoginRes::ERROR);
    userRes->set_token("2 make token error!");
    return false;
  }
  std::string token = rows[0][0].data;
  userRes->set_res(cmsg::CLoginRes::OK);
  userRes->set_token(token);
  userRes->set_expired_time(expiredTime);

  // ������ڵĵ�¼��Ϣ
  ss.str("");
  ss << "delete from cms_token where expired_time<" << now;
  if (!mysql.query(ss.str().c_str())) {
    std::cout << "delete from cms_token failed!\n";
  }

  return true;
}