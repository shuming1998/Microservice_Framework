#pragma once
class CTask {
public:
  struct event_base *base_ = nullptr;
  int sock_ = 0;
  int threadId_ = 0;

  // ��ʼ������
  virtual bool init() = 0;
};

