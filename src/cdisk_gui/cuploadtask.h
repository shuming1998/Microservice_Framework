#ifndef CUPLOAD_H
#define CUPLOAD_H

#include "ccometask.h"
#include <fstream>

typedef void(*uploadCbFunc)();

class CUploadTask : public CComeTask {
public:
  // ���յ���Ϣ������Ϣ�Ļص�
  bool readCb(const CMsg *msg) override;

  // ���ӳɹ�����Ϣ�ص�
  void connetedCb() override;

  // д���ļ��ص�
  void writeCb() override;

  //  �ϴ��ɹ���Ļص�����
  uploadCbFunc uploadCb_ = nullptr;

  void setFilePath(std::string path) { this->filePath_ = path; }

private:
  std::string filePath_ = ""; // ��Ҫ�ϴ����ļ�·��
  std::ifstream ifs_;         // ��ȡ�ļ�
  int fileSize_ = 0;          // �ļ���С
};

#endif // !CUPLOAD_H

