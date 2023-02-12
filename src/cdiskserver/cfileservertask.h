#ifndef CFILE_SERVER_TASK_H
#define CFILE_SERVER_TASK_H

#include "ccometask.h"
#include <fstream>
#include <mutex>

class CFileServerTask : public CComeTask {
public:
  // ���յ���Ϣ�Ļص�
  bool readCb(const CMsg *msg) override;

  // ��������Ϣʱ���ã����ڽ��տͻ��˷��͵��ļ�
  void readCb(void *data, int size) override;

  // д���ļ��ص�
  void writeCb() override;

  static void setCurDir(std::string dir) {
    std::lock_guard<std::mutex> guard(curDirMtx_);
    curDir_ = dir;
  }

  static std::string getCurDir() {
    std::lock_guard<std::mutex> guard(curDirMtx_);
    return std::string(curDir_);
  }

private:
  // �����ȡĿ¼����Ϣ������Ŀ¼�б�
  void getDir(const CMsg *msg);

  // ����ͻ����ϴ�����
  void uploadFile(const CMsg *msg);

  // ����ͻ�����������
  void downloadFile(const CMsg *msg);


  int fileSize_ = 0;            // �ļ���С
  int recvSize_ = 0;            // �ͻ������ϴ��ļ���С
  std::string filePath_;        // �ļ�·��
  std::ofstream ofs_;           // ����д���ļ�
  std::ifstream ifs_;           // ���ڶ�ȡ�ļ�

  static std::mutex curDirMtx_;
  static std::string curDir_;   // ��ǰ·��
  
};

#endif // !CFILE_SERVER_TASK_H

