#pragma once
#include "CTask.h"

class CFtpServerCmd : public CTask {
public:
  CFtpServerCmd() {}
  ~CFtpServerCmd() {}

  // ��ʼ������
  bool init() override;
};

