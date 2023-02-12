#include "cserviceclient.h"


CServiceClient::CServiceClient() {
  threadPool_ = CThreadPoolFactory::create();
}

CServiceClient::~CServiceClient() {
  //delete threadPool_;
  //threadPool_ = nullptr;

}

// ��������뵽�̳߳��У���������
void CServiceClient::startConnect() {
  threadPool_->init(1);
  threadPool_->dispatch(this);
  // �ͻ�����Ҫ���������Զ�����
  setAutoDelete(false);
}