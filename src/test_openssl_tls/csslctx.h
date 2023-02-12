#ifndef CSSL_CTX_H
#define CSSL_CTX_H
#include "cssl.h"


class CSSLCtx {
public:
  // ��ʼ�������
  // @param crtFile �����֤���ļ�
  // @param keyFile �����˽Կ�ļ�
  // @param caFile  ��֤�ͻ���֤��(��ѡ)
  virtual bool initServer(const char *crtFile, const char *keyFile, const char *caFile = nullptr);


  // ��ʼ���ͻ���
  // @param caFile ��֤�����֤��
  virtual bool initClient(const char *caFile = nullptr);


  // ���� SSL ͨ�Ŷ���socket �� ssl_st ��Դ�ɵ������ͷ�
  // �Ƿ񴴽�ʧ��ͨ�� CSSL::isEmpty() �ж�
  CSSL newCSSL(int socket);

private:
  // ��֤�Է�֤��
  void verify(const char *caCrt);
  struct ssl_ctx_st *sslCtx_ = nullptr;
};

#endif // !CSSL_CTX_H

