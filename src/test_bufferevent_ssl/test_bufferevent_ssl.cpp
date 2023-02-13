#include "csslctx.h"

#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <event2/listener.h>
#include <event2/bufferevent_ssl.h>

#include <iostream>
#include <string>
#include <sstream>
#include <string.h>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif // _WIN32

static void readCb(bufferevent *bev, void *arg) {
  static int i = 0;
  char buf[1024] = { 0 };
  int len = bufferevent_read(bev, buf, sizeof(buf) - 1);
  if (len > 0) {
    std::cout << buf << '\n';
  }
  std::string data = "bufferevent client send ";
  data += std::to_string(i++);
  data += "\n";
  bufferevent_write(bev, data.c_str(), data.size());
}

static void writeCb(bufferevent *bev, void *arg) {

}

static void serverReadCb(bufferevent *bev, void *arg) {
  static int i = 0;
  char buf[1024] = { 0 };
  int len = bufferevent_read(bev, buf, sizeof(buf) - 1);
  if (len > 0) {
    std::cout << buf << '\n';
  }
  std::string data = "bufferevent server send ";
  data += std::to_string(i++);
  data += "\n";
  bufferevent_write(bev, data.c_str(), data.size());
}

static void eventCb(bufferevent *bev, short what, void *arg) {
  // ssl ���ֳɹ����ͻ��˺ͷ���˶�����룬��ʾЭ����Կ�ɹ���
  static int i = 0;
  if (what & BEV_EVENT_CONNECTED) {
    std::string data = "bufferevent client send ";
    data += std::to_string(i++);
    data += "\n";
    bufferevent_write(bev, data.c_str(), data.size());
  }
}


static void serverEventCb(bufferevent *bev, short what, void *arg) {
  // ssl ���ֳɹ����ͻ��˺ͷ���˶�����룬��ʾЭ����Կ�ɹ���
  if (what & BEV_EVENT_CONNECTED) {
    auto ssl = (CSSL *)arg;
    ssl->printCert();
    ssl->printCipher();
  }
}

void listenerCb(struct evconnlistener *ev, evutil_socket_t sock, struct sockaddr *addr, int socklen, void *arg) {
  std::cout << "listenCb\n";
  auto ctx = (CSSLCtx *)arg;
  auto csslTmp = ctx->newCSSL(sock);
  CSSL *cssl = new CSSL();
  *(cssl) = csslTmp;
  auto base = evconnlistener_get_base(ev);
  // ���� ssl �ӿ�
  auto bev = bufferevent_openssl_socket_new(base,
    sock,
    cssl->ssl(),
    BUFFEREVENT_SSL_ACCEPTING,   // ��Ӧ openssl �ͻ���
    BEV_OPT_CLOSE_ON_FREE);       // bufferevent_free ʱ���ͷ� socket �� ssl
  if (!bev) {
    std::cout << "bufferevent_openssl_socket_new failed!\n";
  }
  bufferevent_setcb(bev, serverReadCb, writeCb, serverEventCb, cssl);
  bufferevent_enable(bev, EV_READ | EV_WRITE);
}


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
  if (argc > 2) {
    // �ͻ���
    std::cout << "==========Client==========\n";
    std::string ip = argv[2];
    auto base = event_base_new();
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    evutil_make_socket_nonblocking(sock);
    CSSLCtx ctx;
    ctx.initClient("server.crt");
    auto cssl = ctx.newCSSL(sock);
    // ���� ssl �ӿ�
    auto bev = bufferevent_openssl_socket_new(base,
        sock,
        cssl.ssl(),
        BUFFEREVENT_SSL_CONNECTING,   // ��Ӧ openssl �ͻ���
        BEV_OPT_CLOSE_ON_FREE);       // bufferevent_free ʱ���ͷ� socket �� ssl
    if (!bev) {
      std::cout << "bufferevent_openssl_socket_new failed!\n";
      ctx.close();
      getchar();
      return -1;
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);

    // ���� bufferevent ����
    bufferevent_setcb(bev, readCb, writeCb, eventCb, &cssl);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
    bufferevent_socket_connect(bev, (sockaddr *)&addr, sizeof(addr));
    
    for (;;) {
      event_base_loop(base, EVLOOP_NONBLOCK);
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    ctx.close();
    return 0;
  }

  std::cout << "==========Server==========\n";
  auto base = event_base_new();
  CSSLCtx ctx;
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  auto ev = evconnlistener_new_bind(base, listenerCb, &ctx,
    BEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 10, (sockaddr *)&addr, sizeof(addr));
  if (!ev) {
    std::cout << "bind port: " << port << "failed!\n";
    getchar();
    return 0;
  }
  std::cout << "bind port: " << port << "success!\n";
  ctx.initServer("server.crt", "server.key");
  for (;;) {
    event_base_loop(base, EVLOOP_NONBLOCK);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  ctx.close();
  return 0;
}