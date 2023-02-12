#include "cthread.h"
#include "ctask.h"
#include <thread>
#include <iostream>
#include <event2/event.h>

#ifdef _WIN32
#else
#include <unistd.h>
#endif // _WIN32

//�����߳������¼��Ļص�����
static void notifyCb(int fd, short which, void *arg) {
  CThread *t = (CThread *)arg;
  t->notify(fd, which);
}

void CThread::startThread() {
  setup();
  // �����߳�
  std::thread thread(&CThread::mainFunc, this);

  // ����Ϊ�����߳�
  thread.detach();
}

// �߳���ں���
void CThread::mainFunc() {
  std::cout << id_ << " CThread::mainFunc begin\n";
  if (!base_) {
    std::cerr << "CThread::mainFunc() failed because base_ is none!\n";
    std::cerr << "fix WSAStartup!\n";
    return;
  }
  // ����Ϊ�������ַ���Ϣ
  while (!isExit_) {
    // һ�δ��������Ϣ
    event_base_loop(base_, EVLOOP_NONBLOCK);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  
  event_base_free(base_);
  //event_base_dispatch(base_);
  std::cout << id_ << " CThread::mainFunc end\n";
}

bool CThread::setup() {
  // windows �� socketpair�� linux �� pipe
#ifdef _WIN32
  // ����һ�� socketpair�����Ի���ͨ�ţ�fds[0] ���� fds[1] д
  evutil_socket_t fds[2];
  if (evutil_socketpair(AF_INET, SOCK_STREAM, 0, fds) < 0) {
    std::cout << "evutil_socketpair failed!\n";
    return false;
  }
  // ����Ϊ������
  evutil_make_socket_nonblocking(fds[0]);
  evutil_make_socket_nonblocking(fds[1]);
#else
  // linux �ùܵ������� send/recv�� ֻ�� write/read
  int fds[2];
  if (pipe(fds)) {
    std::cerr << "pipe failed!\n";
    return false;
  }
#endif // _WIN32

  // ��ȡ fd ��󶨵� event �¼��У�д�� fd ��Ҫ����
  notifySendFd_ = fds[1];

  // ���� libevent ������(��ز�����һ���߳���������������) 
  event_config *ev_conf = event_config_new();
  event_config_set_flag(ev_conf, EVENT_BASE_FLAG_NOLOCK);
  this->base_ = event_base_new_with_config(ev_conf);
  if (!base_) {
    std::cerr << "event_base_new_with_config failed!\n";
    return false;
  }

  event_config_free(ev_conf);

  // ��ӹܵ������¼������ڼ����߳�ִ������
  event *ev = event_new(base_, fds[0], EV_READ | EV_PERSIST, notifyCb, this);
  event_add(ev, 0);

  return true;
}

void CThread::notify(int fd, short which) {
  // ˮƽ������ ֻҪû�н�����ɣ����ٴν���
  char buf[2] = { 0 };

#ifdef _WIN32
  // ��һ���ַ�
  int res = recv(fd, buf, 1, 0);
#else
  // linux ���ǹܵ���ֻ���� read
  int res = read(fd, buf, 1);
#endif // _WIN32

  if (res <= 0) {
    return;
  }
  std::cout << id_ << " thread " << buf << '\n';

  // ��ȡ����ʼ������
  CTask *task = nullptr;
  taskMutex_.lock();
  if (tasks_.empty()) {
    taskMutex_.unlock();
    return;
  }
  task = tasks_.front();
  tasks_.pop_front();
  taskMutex_.unlock();
  task->init();
}

void CThread::addTask(CTask *task) {
  if (!task) {
    return;
  }

  task->setBase(this->base_);

  //std::lock_guard<std::mutex> guard(taskMutex_);
  taskMutex_.lock();
  tasks_.push_back(task);
  taskMutex_.unlock();
}

// ���յ�һ���ͻ�������֮�󣬾���Ҫ���ô˺��������߳�
void CThread::activate() {
#ifdef _WIN32
  int res = send(this->notifySendFd_, "c", 1, 0);
#else
  // linux ���ǹܵ���ֻ���� read
  int res = write(this->notifySendFd_, "c", 1);
#endif // _WIN32
  if (res <= 0) {
    std::cerr << "CThread::activate  failed!\n";
  }
}