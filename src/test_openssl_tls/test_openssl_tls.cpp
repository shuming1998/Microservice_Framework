#include "csslctx.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif // _WIN32
#include <iostream>
#include <string>
#include <string.h>

int main(int argc, char *argv[]) {
#ifdef _WIN32
  WSADATA wsa;
  WSAStartup(MAKEWORD(2, 2), &wsa);
#endif // _WIN32
  // �ͻ���: port ip
  // �����: port
  int port = 20030;
  if (argc > 1) {
    // �����
    port = atoi(argv[1]);
  }
  if (argc >  2) {
    // �ͻ���
    std::cout << "==========Client==========\n";
    std::string ip = argv[2];

    // ��ʼ���ͻ��� ssl ������
    CSSLCtx clientCtx;
    // ��������ǩ����ֱ���� server�� cacrt ����
    clientCtx.initClient("server.crt");

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);
    int res = connect(sock, (sockaddr *)&addr, sizeof(addr));
    if (res != 0) {
      std::cerr << "connect " << ip << ":" << port << " failed!\n";
      return -1;
    }
    std::cerr << "connect " << ip << ":" << port << " success!\n";

    auto cssl = clientCtx.newCSSL(sock);
    cssl.connect();

    getchar();
    return 0;
  }

  std::cout << "==========Server==========\n";

  // ��ʼ������� ssl ������
  CSSLCtx ctx;
  if (!ctx.initServer("server.crt", "server.key", "server.crt")) {
    std::cerr << "ctx.initServer(\"server.crt\", \"server.key\") failed!\n";
    getchar();
    return -1;
  }
  std::cout << "ctx.initServer(\"server.crt\", \"server.key\") success!\n";

  // �����ͨ��
  int acceptSock = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in serverAddr;
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(port);

  int res = ::bind(acceptSock, (sockaddr *)&serverAddr, sizeof(serverAddr));
  if (res != 0) {
    std::cerr << "bind port: " << port << " failed!\n";
    getchar();
  }

  listen(acceptSock, 10);
  std::cout << "start listen port " << port << '\n';

  for (;;) {
    int clientSocket = accept(acceptSock, 0, 0);
    if (clientSocket <= 0) {
      break;
    }

    // ���� ssl ����
    auto cssl = ctx.newCSSL(clientSocket);
    if (cssl.isEmpty()) {
      std::cout << "cssl isEmpty!\n";
      continue;
    }
    if (!cssl.accept()) {
      continue;
    }
    std::cout << "accept socket\n";
  }

  return 0;
}