#ifndef CSSL_CTX_H
#define CSSL_CTX_H

class CSSLCtx {
public:
  // ��ʼ�������
  // @param crtFile �����֤���ļ�
  // @param keyFile �����˽Կ�ļ�
  // @param caFile  ��֤�ͻ���֤��(��ѡ)
  bool initServer(const char *crtFile, const char *keyFile, const char *caFile = nullptr);

private:
  struct ssl_ctx_st *sslCtx_ = nullptr;
};

#endif // !CSSL_CTX_H

