#include "cconfigclient.h"
#include "cmsgcom.pb.h"
#include "ctools.h"
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>
#include <map>
#include <thread>

#define PB_ROOT "root/"

// ������ʾ�������﷨����
class ParseError : public google::protobuf::compiler::MultiFileErrorCollector {
public:
  virtual void AddError(const std::string& filename, int line, int column,
    const std::string& message) override {
    std::stringstream ss;
    ss << '[' << filename << "|" << line << "|" << column << "]: " << message;
    LOG_DEBUG(ss.str().c_str());
  }
};
static ParseError psError;

// ���ڴ洢�ӷ�������ص�������Ϣ��ҵ���ȡ����ʱ�Ӵ˴���ѯ key: ip:port
static std::map<std::string, cmsg::CConfig> configMap;
static std::mutex configMapMtx;

// �����洢��ǰ΢��������
static google::protobuf::Message *curServiceConfig = nullptr;
static std::mutex curServiceConfigMtx;

// �����洢��ȡ��ȫ�������б�
static cmsg::CConfigList *allConfigs = nullptr;
static std::mutex allConfigsMtx;

CConfigClient::CConfigClient() {
  // �����ļ�����·��
  sourceTree_ = new google::protobuf::compiler::DiskSourceTree();
  sourceTree_->MapPath("", "");
  // ʹ�þ���·��ʱ������ root ��ʧ��
  sourceTree_->MapPath(PB_ROOT, "");
}

void CConfigClient::setCurServiceMessage(google::protobuf::Message *msg) {
  std::lock_guard<std::mutex> guard(curServiceConfigMtx);
  curServiceConfig = msg;
}

google::protobuf::Message *CConfigClient::loadProto(std::string fileName, std::string className, std::string &outProtoCode) {
  // 1 ���� proto �ļ�
  // ��Ҫ����ռ�
  if (importer_) {
    delete importer_;
  }
  importer_ = new google::protobuf::compiler::Importer(sourceTree_, &psError);
  if (!importer_) {
    return nullptr;
  }

  std::string path = PB_ROOT;
  path += fileName;
  // ���� proto �ļ�������
  auto fileDesc = importer_->Import(path);
  if (!fileDesc) {
    return nullptr;
  }
  LOG_DEBUG(fileDesc->DebugString());
  std::stringstream ss;
  ss << fileName << "proto �ļ����سɹ�";
  LOG_DEBUG(ss.str().c_str());

  // ��ȡ����������
  // ��� className Ϊ�գ���ʹ�õ�һ������
  const google::protobuf::Descriptor *messageDesc = nullptr;
  if (className.empty()) {
    if (fileDesc->message_type_count() <= 0) {
      LOG_DEBUG("proto �ļ���û�� message!");
      return nullptr;
    }

    // ȡ��һ������
    messageDesc = fileDesc->message_type(0);
  } else {
    // ���������ռ������
    std::string classNamePack = "";
    // className ��Ϊ����������ͣ��漰�������ռ�����⣬�ж��û��Ƿ����������ռ�
    // ����û�û�д��������ռ�
    if (className.find('.') == className.npos) {
      // �ж�һ�� proto �ļ��Ƿ��������ռ�
      if (fileDesc->package().empty()) {
        classNamePack = className;
      } else {
        classNamePack = fileDesc->package();
        classNamePack += ".";
        classNamePack += className;
      }
    } else {
      // �û������������ռ䣬ֱ�Ӹ�ֵ
      classNamePack = className;
    }
    messageDesc = importer_->pool()->FindMessageTypeByName(classNamePack);
    if (!messageDesc) {
      std::string log = "proto �ļ���û��ָ���� message: ";
      log += classNamePack;
      LOG_DEBUG(log.c_str());
      return nullptr;
    }
  }

  LOG_DEBUG(messageDesc->DebugString().c_str());

  // �������� message ����
  if (message_) {
    delete message_;
  }
  // ��̬������Ϣ���͵Ĺ������������٣��������������� message ����Ҳ������
  static google::protobuf::DynamicMessageFactory factory;
  // �ȴ���һ������ԭ��
  auto messageProto = factory.GetPrototype(messageDesc);
  message_ = messageProto->New();
  // Ҫ�������ݿ�� proto �ļ����ݸ�ʽ��

  /*
  syntax="proto3";
  package xmsg;
  message CDirConfig {
    string root = 1;

  }
  */

  // syntax="proto3";
  outProtoCode = "syntax=\"";
  outProtoCode += fileDesc->SyntaxName(fileDesc->syntax());
  outProtoCode += "\";\n";

  //  package �����ռ�;
  outProtoCode += "package ";
  outProtoCode += fileDesc->package();
  outProtoCode += ";\n";

  // ��ö�ٶ��壬��ʱ��֧��import �� proto �ļ�
  // ͬһ������ֻ����һ�δ���
  std::map<std::string, const google::protobuf::EnumDescriptor *> enumDesc;
  for (int i = 0; i < messageDesc->field_count(); ++i) {
    auto field = messageDesc->field(i);
    if (field->type() != google::protobuf::FieldDescriptor::TYPE_ENUM) {
      continue;
    }
    // �Ѿ���ӹ���ö������
    if (enumDesc.find(field->enum_type()->name()) != enumDesc.end()) {
      continue;
    }
    // �ҵ�ö�ٵ�����
    outProtoCode += field->enum_type()->DebugString() + "\n";
    enumDesc[field->enum_type()->name()] = field->enum_type();
  }

  // message ����
  outProtoCode += messageDesc->DebugString();
  return message_;
}

bool CConfigClient::startGetConfig(const char *serverIp, int serverPort, 
                                   const char *localIp, int localPort,
                                   google::protobuf::Message *configMsg, 
                                   int timeoutSec) {

  regMsgCallback();
  setServerIp(serverIp);
  setServerPort(serverPort);
  if (localIp) {
    strncpy(localIp_, localIp, 16);
  }
  localPort_ = localPort;
  setCurServiceMessage(configMsg);

  startConnect();
  if (!waitforConnected(timeoutSec)) {
    LOG_DEBUG("������������ʧ��!");
    return false;
  }

  LOG_DEBUG("�����������ĳɹ�!");
  // ���ӳɹ���������ʱ��(3000����)
  setTimer(3000);
  
  return true;
}

void CConfigClient::timerCb() {
  // ������ȡ���õ�����
  if (localPort_ > 0) {
    downloadConfig(localIp_, localPort_);
  }
}

int CConfigClient::getInt(const char *key) {
  std::lock_guard<std::mutex> guard(curServiceConfigMtx);
  if (!curServiceConfig) {
    return 0;
  }
  // �Ȼ�ȡ�ֶ�����
  auto field = curServiceConfig->GetDescriptor()->FindFieldByName(key);
  if (!field) {
    return 0;
  }

  // ��ͨ����������ȡ����ֵ
  return curServiceConfig->GetReflection()->GetInt32(*curServiceConfig, field);
}

std::string CConfigClient::getString(const char *key) {
  std::lock_guard<std::mutex> guard(curServiceConfigMtx);
  if (!curServiceConfig) {
    return std::string("");
  }

  // �Ȼ�ȡ�ֶ�����
  auto field = curServiceConfig->GetDescriptor()->FindFieldByName(key);
  if (!field) {
    return std::string("");
  }
  
  // ��ͨ����������ȡ����ֵ
  return curServiceConfig->GetReflection()->GetString(*curServiceConfig, field);
}

bool CConfigClient::getBool(const char *key) {
  std::lock_guard<std::mutex> guard(curServiceConfigMtx);
  if (!curServiceConfig) {
    return false;
  }

  // �Ȼ�ȡ�ֶ�����
  auto field = curServiceConfig->GetDescriptor()->FindFieldByName(key);
  if (!field) {
    return false;
  }

  // ��ͨ����������ȡ����ֵ
  return curServiceConfig->GetReflection()->GetBool(*curServiceConfig, field);
}

bool CConfigClient::getConfig(const char *ip, int port, cmsg::CConfig *outConf) {
  std::stringstream key;
  key << ip << ':' << port;
  std::lock_guard<std::mutex> guard(configMapMtx);
  // ��������
  auto confIt = configMap.find(key.str());
  if (confIt == configMap.end()) {
    LOG_DEBUG("config not find!");
    return false;
  }
  
  // ��������
  outConf->CopyFrom(confIt->second);
  return true;
}

void CConfigClient::wait() {
  CThreadPool::wait();
}

void CConfigClient::uploadConfig(cmsg::CConfig *conf) {
  LOG_DEBUG("�ϴ�������Ϣ");
  sendMsg(cmsg::MSG_UPLOAD_CONFIG_REQ, conf);
}

void CConfigClient::uploadConfigRes(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("�ϴ�������Ϣ��Ӧ");
  cmsg::CMessageRes res;
  if (!res.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("ParseFromArray failed!");
    if (uploadConfigResCb_) {
      uploadConfigResCb_(false, "ParseFromArray failed!");
    }
    return;
  }

  if (res.return_() == cmsg::CMessageRes_CReturn_OK) {
    LOG_DEBUG("�ϴ����óɹ�");
    if (uploadConfigResCb_) {
      uploadConfigResCb_(true, "�ϴ����óɹ�");
    }
    return;
  }

  // �ϴ�ʧ��
  std::stringstream ss;
  ss << "�ϴ�����ʧ�ܣ�" << res.msg();
  if (uploadConfigResCb_) {
    uploadConfigResCb_(false, ss.str().c_str());
  }
  LOG_DEBUG(ss.str().c_str());
}


void CConfigClient::downloadConfig(const char *ip, int port) {
  LOG_DEBUG("����������Ϣ");
  if (port <= 0 || port >= 65536) {
    LOG_DEBUG("downloadConfig failed: port out of range!");
    return;
  }

  // ��������������Ϣ
  cmsg::CDownloadconfigReq req;
  if (ip) {
    req.set_serviceip(ip);
  }
  req.set_serviceport(port);
  // ������Ϣ�������
  sendMsg(cmsg::MSG_DOWNLOAD_CONFIG_REQ, &req);
}

void CConfigClient::downloadConfigRes(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("����������Ϣ��Ӧ");
  cmsg::CConfig conf;
  if (!conf.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("downloadConfigRes ParseFromArray failed!");
    return;
  }
  LOG_DEBUG(conf.DebugString().c_str());

  std::stringstream key;
  key << conf.serviceip() << ':' << conf.serviceport();
  {
    std::lock_guard<std::mutex> guard(configMapMtx);
    configMap[key.str()] = conf;
  }

  // �洢��������
  if (localPort_ > 0 && curServiceConfig) {
    std::stringstream localKey;
    localKey << conf.serviceip() << ':' << localPort_;
    // ȷ���Ǳ��ص�������
    if (key.str() == localKey.str()) {
      LOG_DEBUG("=====&&&&=====");
      std::lock_guard<std::mutex> guard(curServiceConfigMtx);
      if (curServiceConfig) {
        curServiceConfig->ParseFromString(conf.privatepb());
      }
    }
  }
}

cmsg::CConfigList CConfigClient::downloadAllConfig(int page, int pageCount, int timeoutSec) {
  {
    // ��������ʷ����
    std::lock_guard<std::mutex> guard(allConfigsMtx);
    delete allConfigs;
    allConfigs = nullptr;
  }
  
  cmsg::CConfigList configs;
  // 1 �Ͽ�����ʱ�Զ�����
  if (!autoConnect(timeoutSec)) {
    return configs;
  }

  // 2 ���ͻ�ȡȫ�����õ�������Ϣ
  cmsg::CDownloadAllConfigReq req;
  req.set_page(page);
  req.set_pagecount(pageCount);
  sendMsg(cmsg::MSG_DOWNLOAD_ALL_CONFIG_REQ, &req);


  // 3 ÿ 10 �������һ���Ƿ��ܵ���Ӧ
  int milsec = timeoutSec * 100;
  for (int i = 0; i < milsec; ++i) {
    {
      std::lock_guard<std::mutex> guard(allConfigsMtx);
      if (allConfigs) {
        return *allConfigs;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return configs;
}

void CConfigClient::downloadAllConfigRes(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("��ȡȫ�������б���Ӧ");
  std::lock_guard<std::mutex> guard(allConfigsMtx);
  if (!allConfigs) {
    allConfigs = new cmsg::CConfigList();
  }
  allConfigs->ParseFromArray(msg->data_, msg->size_);


}

void CConfigClient::deleteConfig(const char *ip, int port) {
  if (!ip || strlen(ip) == 0 || port < 0 || port >= 65536) {
    LOG_DEBUG("deleteConfig failed: ip or port not invalid!");
    return;
  }

  // ����ɾ��������Ϣ
  cmsg::CDownloadconfigReq req;
  if (ip) {
    req.set_serviceip(ip);
  }
  req.set_serviceport(port);
  // ������Ϣ�������
  sendMsg(cmsg::MSG_DELETE_CONFIG_REQ, &req);
}

void CConfigClient::deleteConfigRes(cmsg::CMsgHead *head, CMsg *msg) {
  LOG_DEBUG("ɾ��������Ӧ");
  cmsg::CMessageRes res;
  if (!res.ParseFromArray(msg->data_, msg->size_)) {
    LOG_DEBUG("deleteConfigRes ParseFromArray failed!");
    return;
  }

  if (res.return_() == cmsg::CMessageRes_CReturn_OK) {
    LOG_DEBUG("ɾ�����óɹ�");
    return;
  }

  LOG_DEBUG("ɾ������ʧ��");
}