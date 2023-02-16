#include "clogclient.h"
#include "configdao.h"
#include "cmysql.h"
#include "ctools.h"
#include "cmsg.h"

#define CONFIG_TABLE "cms_service_config"

// ȷ����װ����ȡ���á�д��/��������ʱ ���ݿ���̰߳�ȫ
static std::mutex mysql_Mtx;

bool ConfigDAO::init(const char *ip, const char *user, const char *password, const char *db, int port) {
  std::lock_guard<std::mutex> guard(mysql_Mtx);
  if (!mysql_) {
    mysql_ = new cmysql::CMysql();
  }

  if (!mysql_->init()) {
    LOG_DEBUG("mysql_->init() failed!");
    return false;
  }

  // ���� mysql �Զ�����
  mysql_->setReconnect(true);
  // ���� mysql ��ȴ�����ʱ��
  mysql_->setConnectTimeout(3);
  if (!mysql_->connect(ip, user, password, db, port)) {
    LOG_DEBUG("mysql_->connect failed!");
    return false;
  }

  LOG_DEBUG("mysql_->connect success!");

  return true;
}

bool ConfigDAO::install() {
  LOG_DEBUG("ConfigDAO::install()!");
  std::lock_guard<std::mutex> guard(mysql_Mtx);
  if (!mysql_) {
    LOG_ERROR("mysql_ not init!");
    return false;
  }

  // ��������ڣ��򴴽���
  std::string createTbSql = "CREATE TABLE IF NOT EXISTS `cms_service_config` ( \
                            `id` INT AUTO_INCREMENT, \
                            `service_name` VARCHAR(16), \
                            `service_port` INT, \
                            `service_ip` VARCHAR(16), \
                            `private_pb` VARCHAR(4096), \
                            `proto` VARCHAR(4096), \
                            PRIMARY KEY(`id`)); ";
  if (!mysql_->query(createTbSql.c_str())) {
    LOG_INFO("CREATE TABLE cms_service_config failed!\n");
    return false;
  }

  LOG_INFO("CREATE TABLE cms_service_config sucess!\n");
  return true;
}

bool ConfigDAO::uploadConfig(cmsg::CConfig *conf) {
  LOG_DEBUG("ConfigDAO::uploadConfig!");

  std::lock_guard<std::mutex> guard(mysql_Mtx);
  if (!mysql_) {
    LOG_ERROR("mysql_ not init!");
    return false;
  }

  if (!conf || conf->serviceip().empty()) {
    LOG_ERROR("ConfigDAO::saveConfig failed!");
    return false;
  }

  // ׼������
  std::string tb = CONFIG_TABLE;
  cmysql::MData data;
  data["service_name"] = cmysql::CData(conf->servicename().c_str());
  int port = conf->serviceport();
  data["service_port"] = cmysql::CData(&port);
  data["service_ip"] = cmysql::CData(conf->serviceip().c_str());
  // �����л�һ�Σ������� CConfig ���뵽 private_pb
  std::string privatePb;
  conf->SerializeToString(&privatePb);
  data["private_pb"].data = privatePb.c_str();
  data["private_pb"].size = privatePb.size();
  data["proto"].data = conf->proto().c_str();
  data["proto"].size = conf->proto().size();

  // ����Ѿ��д������ݣ����޸�����
  std::stringstream ss;
  ss << " where service_ip='";
  ss << conf->serviceip() << "' and service_port=" << conf->serviceport();
  std::string where = ss.str();
  std::string sql = "select id from ";
  sql += tb;
  sql += where;

  LOG_DEBUG(sql);
  auto rows = mysql_->getResult(sql.c_str());
  bool res;
  if (rows.size() > 0) {
    int affetcRows  =  mysql_->updateBin(data, tb, where);
    if (affetcRows >= 0) {
      LOG_DEBUG("���ø��³ɹ�!");
      return true;
    }
    
    LOG_DEBUG("���ø���ʧ��!");
    return false;
  }

  // ��������
  res = mysql_->insertBin(data, tb);
  if (res) {
    LOG_DEBUG("���ò���ɹ�!");
  } else {
    LOG_DEBUG("���ò���ʧ��!");
  }
  return res;
}

cmsg::CConfig ConfigDAO::downloadConfig(const char *ip, int port) {
  cmsg::CConfig conf;
  LOG_DEBUG("ConfigDAO::loadConfig!");

  std::lock_guard<std::mutex> guard(mysql_Mtx);
  if (!mysql_) {
    LOG_ERROR("mysql_ not init!");
    return conf;
  }

  if (!ip || port <= 0 || port >= 65536 || strlen(ip) == 0) {
    LOG_ERROR("ConfigDAO::loadConfig failed: ip or port is invalid!");
    return conf;
  }

  std::string tb = CONFIG_TABLE;
  std::stringstream ss;
  // ֻ��Ҫ��ȡ������
  ss << "select private_pb from " << tb;
  ss << " where service_ip='" << ip << "' and service_port=" << port;
  auto rows = mysql_->getResult(ss.str().c_str());
  conf.set_serviceip(ip);
  conf.set_serviceport(port);
  if (rows.size() == 0) {
    LOG_DEBUG("download config failed!");
    return conf;
  }

  // ֻȡ��һ����¼
  auto row = rows[0];
  if (!conf.ParseFromArray(row[0].data, row[0].size)) {
    LOG_DEBUG("download config failed��ParseFromArray failed!");
    return conf;
  }

  LOG_DEBUG("download config success!");
  LOG_DEBUG(conf.DebugString());
  return conf;
}

cmsg::CConfigList ConfigDAO::downloadAllConfig(int page, int pageCount) {
  cmsg::CConfigList configs;
  LOG_DEBUG("downloadAllConfig");
  std::lock_guard<std::mutex> guard(mysql_Mtx);
  if (!mysql_) {
    LOG_ERROR("mysql_ not init!");
    return configs;
  }
  if (page <= 0 || pageCount <= 0) {
    LOG_ERROR("downloadAllConfig error: page or pageCount invalid!");
    return configs;
  }

  // ��ȡ��ҳ�������б� page �� 1 ��ʼ ÿҳ���� pageCount 
  std::string tableName = CONFIG_TABLE;
  std::stringstream ss;
  ss << "select `service_name`,`service_ip`,`service_port` from " << tableName;
  // ��ҳ���� select * from table limit 0,10��10,10��20,10��.....��
  ss << " order by id desc";
  ss << " limit " << (page - 1) * pageCount << "," << pageCount;
  LOG_DEBUG(ss.str().c_str());
  auto rows = mysql_->getResult(ss.str().c_str());
  for (auto row : rows) {
    // ������������뵽 proto ������
    auto pConf = configs.add_config();
    pConf->set_servicename(row[0].data);
    pConf->set_serviceip(row[1].data);
    pConf->set_serviceport(atoi(row[2].data));
  }

  return configs;
}

bool ConfigDAO::deleteConfig(const char *ip, int port) {
  LOG_DEBUG("deleteConfig");
  std::lock_guard<std::mutex> guard(mysql_Mtx);
  if (!mysql_) {
    LOG_ERROR("mysql_ not init!");
    return false;
  }

  if (!ip || port <= 0 || port >= 65536 || strlen(ip) == 0) {
    LOG_ERROR("ConfigDAO::deleteConfig failed: ip or port is invalid!");
    return false;
  }

  std::string tb = CONFIG_TABLE;
  std::stringstream ss;
  ss << "delete from " << tb;
  ss << " where service_ip='" << ip << "' and service_port=" << port;

  return mysql_->query(ss.str().c_str());
}