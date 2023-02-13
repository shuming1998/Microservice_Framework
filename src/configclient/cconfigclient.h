#ifndef CCONFIG_CLIENT_H
#define CCONFIG_CLIENT_H
#include "cserviceclient.h"
#include "cmsgcom.pb.h"

namespace google {
  namespace protobuf {
    namespace compiler {
      class Importer;
      class DiskSourceTree;
    }
  }
}

typedef void(*uploadConfigResCbFunc)(bool isOk, const char *msg);

class CConfigClient : public CServiceClient {
public:
  static CConfigClient *get() {
    static CConfigClient cc;
    return &cc;
  }

  // �����������ģ�������ʱ����ȡ����
  // ��װ��regMsgCallback��setServerIp��setServerPort��startConnect����ʱ�ȴ�����
  bool startGetConfig(const char *serverIp, int serverPort, 
                      const char *localIp, int localPort,
                      google::protobuf::Message *configMsg, int timeoutSec = 10);

  // ��ʱ�����õĻص�����
  void timerCb();

  // ��ҵ����ã�ͨ���˽ӿڻ�ȡ����
  bool getConfig(const char *ip, int port, cmsg::CConfig *outConf);

  // ��ȡ���صı��ز���
  int getInt(const char *key);
  std::string getString(const char *key);
  bool getBool(const char *key);   // isSSL

  // �ϴ���������
  void uploadConfig(cmsg::CConfig *conf);
  // �ϴ������������Ӧ
  void uploadConfigRes(cmsg::CMsgHead *head, CMsg *msg);

  // ������������
  // @param ip ���Ϊ NULL����ȡ�ͻ������ӵ��������ĵĵ�ַ
  void downloadConfig(const char *ip, int port);
  // ���������������Ӧ
  void downloadConfigRes(cmsg::CMsgHead *head, CMsg *msg);

  // ����ȫ�������б����� 1 �Ͽ�����ʱ�Զ����� 2 �����ȴ��������
  cmsg::CConfigList downloadAllConfig(int page, int pageCount, int timeoutSec);
  // ����ȫ�������������Ӧ
  void downloadAllConfigRes(cmsg::CMsgHead *head, CMsg *msg);

  // ɾ��ָ��΢��������
  void deleteConfig(const char *ip, int port);
  // ɾ��ָ��΢�������õ���Ӧ
  void deleteConfigRes(cmsg::CMsgHead *head, CMsg *msg);

  // ���õ�ǰ�����ö���(����)
  void setCurServiceMessage(google::protobuf::Message *msg);

  // ���� proto �ļ�,���̰߳�ȫ
  // @param fileName �ļ�·��
  // @param className �ļ�����
  // @param outProtoCode ��ȡ���Ĵ��룬���������ռ�Ͱ汾
  // @return ���ض�̬��������Ϣ��ʧ�ܷ��� nullptr���ڶ��ε��û��ͷ���һ�εĿռ�
  google::protobuf::Message *loadProto(std::string fileName, std::string className, std::string &outProtoCode);

  // ע��ص�����
  static void regMsgCallback() {
    regCb(cmsg::MSG_UPLOAD_CONFIG_RES, (msgCbFunc)&CConfigClient::uploadConfigRes);
    regCb(cmsg::MSG_DOWNLOAD_CONFIG_RES, (msgCbFunc)&CConfigClient::downloadConfigRes);
    regCb(cmsg::MSG_DOWNLOAD_ALL_CONFIG_RES, (msgCbFunc)&CConfigClient::downloadAllConfigRes);
    regCb(cmsg::MSG_DELETE_CONFIG_RES, (msgCbFunc)&CConfigClient::deleteConfigRes);
  }

  // �ȴ��߳��˳�
  void wait();

  // �ϴ����óɹ������ϴ����õ���Ӧ�е��ã�����ˢ���ϴ��ɹ���Ľ���
  uploadConfigResCbFunc uploadConfigResCb_ = nullptr;

private:
  CConfigClient();

  char localIp_[16] = { 0 };  // ����΢����� IP
  int localPort_ = 0;          // ����΢����Ķ˿ں�

  // ��̬���� proto �ļ�
  google::protobuf::compiler::Importer *importer_ = nullptr;
  // �����ļ��Ĺ������
  google::protobuf::compiler::DiskSourceTree *sourceTree_ = nullptr;
  // �������ɵ� message ���󣬸��� proto �ļ���̬����
  google::protobuf::Message *message_ = nullptr;
};

#endif