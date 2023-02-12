#ifndef CDISK_CLIENT_H
#define CDISK_CLIENT_H

#include <QObject>
#include <string>
#include <iostream>

class CDiskClient : public QObject {

  Q_OBJECT

public:
  // ����ģʽ
  static CDiskClient *get() {
    static CDiskClient cc;
    return &cc;
  }

  bool init();

  // ��ȡ������ļ�Ŀ¼
  void getDir();

  // �ϴ��ļ�
  void uploadFile(std::string filePath);
  // �����ļ�
  //@param ������ļ����·��
  //@param ���ش洢Ŀ¼
  void downloadFile(std::string serverPath, std::string localDir);

  void setServerIp(std::string ip) { serverIp_ = ip; }
  void setServerPort(int port) { serverPort_ = port; }
  void setServerRoot(std::string root) { serverRoot_ = root; }

signals:
  void sDir(std::string dir);
  void sUploadComplete();
  void sDownloadComplete();

private:
  std::string serverIp_ = "";   // ������ ip ��ַ
  std::string serverRoot_ = "";  // ������ root Ŀ¼
  int serverPort_ = 0;          // �������˿ں�

  CDiskClient() {}
};



#endif

