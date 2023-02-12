#ifndef CDOWNLOAD_TASK_H
#define CDOWNLOAD_TASK_H


#include "ccometask.h"
#include <fstream>

typedef void(*downloadCbFunc)();

class CDownloadTask : public CComeTask {
public:
  // ���յ���Ϣ�Ļص�
  bool readCb(const CMsg *msg) override;

  // ��������
  void readCb(void *data, int size) override;

  // ���ӳɹ�����Ϣ�ص�
  void connetedCb() override;

  downloadCbFunc downloadCb_ = nullptr;

  void setFilePath(std::string path) { this->filePath_ = path; }
  void setLocalDir(std::string dir) { this->localDir_ = dir; }
  
private:
  int fileSize_ = 0;            // �ļ���С
  int recvSize_ = 0;            // �������ļ���С
  std::string filePath_;   // ��Ҫ���ص��ļ����·��
  std::string localDir_;   // ���ش洢·��
  std::ofstream ofs_;           // ����д���ļ�
};

#endif // !CDOWNLOAD_TASK_H

