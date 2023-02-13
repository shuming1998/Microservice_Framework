#include "csslctx.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <iostream>

// ��֤�Է�֤��Ļص�����
static int verifyCb(int preverify_ok, X509_STORE_CTX *x509_ctx) {
  if (preverify_ok == 0) {
    std::cerr << "SSL cert validate failed!\n";
  } else {
    std::cout << "SSL cert validate success!\n";
  }

  // ��������һ����֤������ֻ����֤�Է���֤���Ƿ��� ca ǩ��
  return preverify_ok;
}

bool CSSLCtx::initServer(const char *crtFile, const char *keyFile, const char *caFile) {
  // ���� ssl ctx ������
  sslCtx_ = SSL_CTX_new(TLS_server_method());
  if (!sslCtx_) {
    std::cerr << "SSL_CTX_new TLS_server_method failed!\n";
    return false;
  }

  // ����֤��,˽Կ����֤
  // ����֤��
  int res = SSL_CTX_use_certificate_file(sslCtx_, crtFile, SSL_FILETYPE_PEM);
  if (res <= 0) {
    ERR_print_errors_fp(stderr);
    return false;
  }
  std::cout << "Load certificate success!\n";

  // ����˽Կ
  res = SSL_CTX_use_PrivateKey_file(sslCtx_, keyFile, SSL_FILETYPE_PEM);
  if (res <= 0) {
    ERR_print_errors_fp(stderr);
    return false;
  }
  std::cout << "Load PrivateKey success!\n";

  // ��֤
  res = SSL_CTX_check_private_key(sslCtx_);
  if (res <= 0) {
    std::cout << "PrivateKey does not match the certificate!";
    return false;
  }
  std::cout << "PrivateKey match the certificate!\n";

  // ��֤�ͻ���֤��
  verify(caFile);
  return true;
}

bool CSSLCtx::initClient(const char *caFile) {
  // ���� ssl ctx ������
  sslCtx_ = SSL_CTX_new(TLS_client_method());
  if (!sslCtx_) {
    std::cerr << "SSL_CTX_new TLS_server_method failed!\n";
    return false;
  }

  // ��֤������֤��
  verify(caFile);

  return true;
}

CSSL CSSLCtx::newCSSL(int socket) {
  CSSL cssl;
  if (socket <= 0 || !sslCtx_) {
    std::cerr << "socket <= 0 || !sslCtx_\n";
    return cssl;
  }

  auto ssl = SSL_new(sslCtx_);
  if (!ssl) {
    std::cerr << "SSL_new failed!\n";
    return cssl;
  }

  // ���� socket
  SSL_set_fd(ssl, socket);
  cssl.setSSL(ssl);
  return cssl;
}

void CSSLCtx::verify(const char *caCrt) {
  if (!caCrt || ! sslCtx_) {
    std::cerr << "skip verify caCrt: caCrt or sslCtx_ is nullptr!\n";
    return;
  }

  // ������֤�Է�֤��
  SSL_CTX_set_verify(sslCtx_, SSL_VERIFY_PEER, verifyCb);
  // ָ��֤��
  SSL_CTX_load_verify_locations(sslCtx_, caCrt, 0);
}

void CSSLCtx::close() {
  if (sslCtx_) {
    SSL_CTX_free(sslCtx_);
    sslCtx_ = nullptr;
  }
}