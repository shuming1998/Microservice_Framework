#include "configgui.h"
#include "cconfigclient.h"
#include "configedit.h"
#include <QtWidgets/QApplication>


int main(int argc, char *argv[]) {
  // ��ʼ�����ÿͻ��ˣ������̳߳�
  CConfigClient::get()->startGetConfig("123.56.142.26", CONFIG_PORT, 0, 0, 0);
  QApplication a(argc, argv);
  //ConfigEdit edit;
  //edit.exec();
  //return 0;
  ConfigGui w;
  std::cout << "===================\n";
  w.show();
  std::cout << "===================\n";
  return a.exec();
}
